/*
Copyright 2011-2017 Frederic Langlet
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
you may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <sstream>
#include "../util.hpp" // Visual Studio min/max
#include "FunctionFactory.hpp"
#include "LZCodec.hpp"

using namespace kanzi;

LZCodec::LZCodec() THROW
{
    _delegate = new LZXCodec();
}

LZCodec::LZCodec(Context& ctx) THROW
{
   int lzpType = ctx.getInt("lz", FunctionFactory<byte>::LZ_TYPE);
    _delegate = (lzpType == FunctionFactory<byte>::LZP_TYPE) ? (Function<byte>*)new LZPCodec() : 
       (Function<byte>*)new LZXCodec();
}

bool LZCodec::forward(SliceArray<byte>& input, SliceArray<byte>& output, int count) THROW
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("ROLZ codec: Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("ROLZ codec: Invalid output block");

    if (input._array == output._array)
        return false;

    return _delegate->forward(input, output, count);
}

bool LZCodec::inverse(SliceArray<byte>& input, SliceArray<byte>& output, int count) THROW
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("ROLZ codec: Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("ROLZ codec: Invalid output block");

    if (input._array == output._array)
        return false;

    return _delegate->inverse(input, output, count);
}

int LZXCodec::emitLastLiterals(const byte src[], byte dst[], int litLen)
{
    int dstIdx = 1;

    if (litLen >= 7) {
        dst[0] = byte(7 << 5);
        dstIdx += emitLength(&dst[1], litLen - 7);
    }
    else {
        dst[0] = byte(litLen << 5);
    }

    memcpy(&dst[dstIdx], src, litLen);
    return dstIdx + litLen;
}

bool LZXCodec::forward(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    if (input._array == output._array)
        return false;

    if (output._length < getMaxEncodedLength(count))
        return false;

    // If too small, skip
    if (count < MIN_LENGTH)
        return false;

    const int srcEnd = count - 8;
    byte* dst = &output._array[output._index];
    byte* src = &input._array[input._index];
    int dstIdx = 0;

    if (_bufferSize == 0) {
        _bufferSize = 1 << HASH_LOG;
        delete[] _hashes;
        _hashes = new int32[_bufferSize];
    }

    memset(_hashes, 0, sizeof(int32) * _bufferSize);
    const int maxDist = (srcEnd < 4 * MAX_DISTANCE1) ? MAX_DISTANCE1 : MAX_DISTANCE2;
    dst[dstIdx++] = (maxDist == MAX_DISTANCE1) ? byte(0) : byte(1);
    int srcIdx = 0;
    int anchor = 0;

    while (srcIdx < srcEnd) {
        const int minRef = max(srcIdx - maxDist, 0);
        const int32 h = hash(&src[srcIdx]);
        const int ref = _hashes[h];
        int bestLen = 0;

        // Find a match
        if ((ref > minRef) && (LZCodec::sameInts(src, ref, srcIdx) == true)) {
            const int maxMatch = srcEnd - srcIdx;
            bestLen = 4;

            while ((bestLen + 4 < maxMatch) && (LZCodec::sameInts(src, ref + bestLen, srcIdx + bestLen) == true))
                bestLen += 4;

            while ((bestLen < maxMatch) && (src[ref + bestLen] == src[srcIdx + bestLen]))
                bestLen++;
        }

        // No good match ?
        if (bestLen < MIN_MATCH) {
            _hashes[h] = srcIdx;
            srcIdx++;
            continue;
        }

        // Emit token
        // Token: 3 bits litLen + 1 bit flag + 4 bits mLen
        // flag = if maxDist = (1<<17)-1, then highest bit of distance
        //        else 1 if dist needs 3 bytes (> 0xFFFF) and 0 otherwise
        const int mLen = bestLen - MIN_MATCH;
        const int dist = srcIdx - ref;
        const int token = ((dist > 0xFFFF) ? 0x10 : 0) | min(mLen, 0x0F);

        // Literals to process ?
        if (anchor == srcIdx) {
            dst[dstIdx++] = byte(token);
        }
        else {
            // Process match
            const int litLen = srcIdx - anchor;

            // Emit literal length
            if (litLen >= 7) {
                dst[dstIdx++] = byte((7 << 5) | token);
                dstIdx += emitLength(&dst[dstIdx], litLen - 7);
            }
            else {
                dst[dstIdx++] = byte((litLen << 5) | token);
            }

            // Emit literals
            emitLiterals(&src[anchor], &dst[dstIdx], litLen);
            dstIdx += litLen;
        }

        // Emit match length
        if (mLen >= 0x0F)
            dstIdx += emitLength(&dst[dstIdx], mLen - 0x0F);

        // Emit distance
        if ((maxDist == MAX_DISTANCE2) && (dist > 0xFFFF))
            dst[dstIdx++] = byte(dist >> 16);

        dst[dstIdx++] = byte(dist >> 8);
        dst[dstIdx++] = byte(dist);

        // Fill _hashes and update positions
        anchor = srcIdx + bestLen;
        _hashes[h] = srcIdx;
        srcIdx++;

        while (srcIdx < anchor) {
            _hashes[hash(&src[srcIdx])] = srcIdx;
            srcIdx++;
        }
    }

    // Emit last literals
    dstIdx += emitLastLiterals(&src[anchor], &dst[dstIdx], srcEnd + 8 - anchor);
    input._index = srcEnd + 8;
    output._index = dstIdx;
    return true;
}

bool LZXCodec::inverse(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    if (input._array == output._array)
        return false;

    const int srcEnd = count - 8;
    const int dstEnd = output._length - 8;
    byte* dst = &output._array[output._index];
    byte* src = &input._array[input._index];
    int dstIdx = 0;
    const int maxDist = (src[0] == byte(1)) ? MAX_DISTANCE2 : MAX_DISTANCE1;
    int srcIdx = 1;

    while (true) {
        const int token = int(src[srcIdx++]);

        if (token >= 32) {
            // Get literal length
            int litLen = token >> 5;

            if (litLen == 7) {
                while ((srcIdx < srcEnd) && (src[srcIdx] == byte(0xFF))) {
                    srcIdx++;
                    litLen += 0xFF;
                }

                if (srcIdx >= srcEnd + 8) {
                    input._index += srcIdx;
                    output._index += dstIdx;
                    return false;
                }

                litLen += int(src[srcIdx++]);
            }

            // Copy literals and exit ?
            if ((dstIdx + litLen > dstEnd) || (srcIdx + litLen > srcEnd)) {
                memcpy(&dst[dstIdx], &src[srcIdx], litLen);
                srcIdx += litLen;
                dstIdx += litLen;
                break;
            }

            // Emit literals
            emitLiterals(&src[srcIdx], &dst[dstIdx], litLen);
            srcIdx += litLen;
            dstIdx += litLen;
        }

        // Get match length
        int mLen = token & 0x0F;

        if (mLen == 15) {
            while ((srcIdx < srcEnd) && (src[srcIdx] == byte(0xFF))) {
                srcIdx++;
                mLen += 0xFF;
            }

            if (srcIdx < srcEnd)
                mLen += int(src[srcIdx++]);
        }

        mLen += MIN_MATCH;
        const int mEnd = dstIdx + mLen;

        // Sanity check
        if (mEnd > dstEnd + 8) {
            input._index += srcIdx;
            output._index += dstIdx;
            return false;
        }

        // Get distance
        int dist = (int(src[srcIdx]) << 8) | int(src[srcIdx + 1]);
        srcIdx += 2;

        if ((token & 0x10) != 0) {
            dist = (maxDist == MAX_DISTANCE1) ? dist + 65536 : (dist << 8) | int(src[srcIdx++]);
        }

        // Sanity check
        if ((dstIdx < dist) || (dist > maxDist)) {
            input._index += srcIdx;
            output._index += dstIdx;
            return false;
        }

        // Copy match
        if (dist > 8) {
            int ref = dstIdx - dist;

            do {
                // No overlap
                memcpy(&dst[dstIdx], &dst[ref], 8);
                ref += 8;
                dstIdx += 8;
            } while (dstIdx < mEnd);
        }
        else {
            const int ref = dstIdx - dist;

            for (int i = 0; i < mLen; i++)
                dst[dstIdx + i] = dst[ref + i];
        }

        dstIdx = mEnd;
    }

    output._index = dstIdx;
    input._index = srcIdx;
    return srcIdx == srcEnd + 8;
}


bool LZPCodec::forward(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    if (input._array == output._array)
        return false;

    if (output._length < getMaxEncodedLength(count))
        return false;

    // If too small, skip
    if (count < MIN_LENGTH)
        return false;

    byte* dst = &output._array[output._index];
    byte* src = &input._array[input._index];
    const int srcEnd = count - 8;
    const int dstEnd = output._length - 4;

    if (_bufferSize == 0) {
        _bufferSize = 1 << HASH_LOG;
        delete[] _hashes;
        _hashes = new int32[_bufferSize];
    }

    memset(_hashes, 0, sizeof(int32) * _bufferSize);
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    int32 ctx = LittleEndian::readInt32(&src[0]);
    int srcIdx = 4;
    int dstIdx = 4;
    int minRef = 4;

    while ((srcIdx < srcEnd) && (dstIdx < dstEnd)) {
        const int32 h = (HASH_SEED * ctx) >> HASH_SHIFT;
        const int32 ref = _hashes[h];
        _hashes[h] = srcIdx;
        int bestLen = 0;

        // Find a match
        if ((ref > minRef) && (LZCodec::sameInts(src, ref, srcIdx) == true)) {
            const int maxMatch = srcEnd - srcIdx;
            bestLen = 4;

            while ((bestLen < maxMatch) && (LZCodec::sameInts(src, ref + bestLen, srcIdx + bestLen) == true))
                bestLen += 4;

            while ((bestLen < maxMatch) && (src[ref + bestLen] == src[srcIdx + bestLen]))
                bestLen++;
        }

        // No good match ?
        if (bestLen < MIN_MATCH) {
            const int val = int32(src[srcIdx]);
            ctx = (ctx << 8) | val;
            dst[dstIdx++] = src[srcIdx++];

            if (ref != 0) {
                if (val == MATCH_FLAG)
                    dst[dstIdx++] = byte(0xFF);

                if (minRef < bestLen)
                    minRef = srcIdx + bestLen;
            }

            continue;
        }

        srcIdx += bestLen;
        ctx = LittleEndian::readInt32(&src[srcIdx - 4]);
        dst[dstIdx++] = byte(MATCH_FLAG);
        bestLen -= MIN_MATCH;

        // Emit match length
        while (bestLen >= 254) {
            bestLen -= 254;
            dst[dstIdx++] = byte(0xFE);

            if (dstIdx >= dstEnd)
                break;
        }

        dst[dstIdx++] = byte(bestLen);
    }

    while ((srcIdx < srcEnd + 8) && (dstIdx < dstEnd)) {
        const int32 h = (HASH_SEED * ctx) >> HASH_SHIFT;
        const int ref = _hashes[h];
        _hashes[h] = srcIdx;
        const int val = int32(src[srcIdx]);
        ctx = (ctx << 8) | val;
        dst[dstIdx++] = src[srcIdx++];

        if ((ref != 0) && (val == MATCH_FLAG) && (dstIdx < dstEnd))
            dst[dstIdx++] = byte(0xFF);
    }

    input._index = srcIdx;
    output._index = dstIdx;
    return (srcIdx == count) && (dstIdx < (count - (count >> 6)));
}


bool LZPCodec::inverse(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    if (input._array == output._array)
        return false;
   
    if (count < 4)
        return false;

    const int srcEnd = count;
    byte* dst = &output._array[output._index];
    byte* src = &input._array[input._index];

    if (_bufferSize == 0) {
        _bufferSize = 1 << HASH_LOG;
        delete[] _hashes;
        _hashes = new int32[_bufferSize];
    }

    memset(_hashes, 0, sizeof(int32) * _bufferSize);
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    int32 ctx = LittleEndian::readInt32(&dst[0]);
    int srcIdx = 4;
    int dstIdx = 4;

    while (srcIdx < srcEnd) {
        const int32 h = (HASH_SEED * ctx) >> HASH_SHIFT;
        const int32 ref = _hashes[h];
        _hashes[h] = dstIdx;

        if ((ref == 0) || (src[srcIdx] != byte(MATCH_FLAG))) {
           dst[dstIdx] = src[srcIdx];
           ctx = (ctx << 8) | int32(dst[dstIdx]);
           srcIdx++;
           dstIdx++;
           continue;
        }

        srcIdx++;

        if (src[srcIdx] == byte(0xFF)) {
           dst[dstIdx] = byte(MATCH_FLAG);
           ctx = (ctx << 8) | int32(MATCH_FLAG);
           srcIdx++;
           dstIdx++;
           continue;
        }

        int mLen = MIN_MATCH;

        while ((srcIdx < srcEnd) && (src[srcIdx] == byte(0xFE))) {
           srcIdx++;
           mLen += 254;
        }
        
        if (srcIdx >= srcEnd)
           break;
        
        mLen += int(src[srcIdx++]);

        for (int i = 0; i < mLen; i++)
           dst[dstIdx + i] = dst[ref + i];

        dstIdx += mLen;
        ctx = LittleEndian::readInt32(&dst[dstIdx - 4]);
    }

    input._index = srcIdx;
    output._index = dstIdx;
    return srcIdx == srcEnd;
}