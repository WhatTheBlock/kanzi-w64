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
#include "LZCodec.hpp"
#include "../Memory.hpp"

using namespace kanzi;

LZCodec::LZCodec()
{
    _buffer = new int[0];
    _bufferSize = 0;
}

int LZCodec::emitLastLiterals(byte src[], byte dst[], int runLength)
{
    int dstIdx = 1;

    if (runLength >= RUN_MASK) {
        dst[0] = byte(RUN_MASK << ML_BITS);
        dstIdx += emitLength(&dst[1], runLength - RUN_MASK);
    }
    else {
        dst[0] = byte(runLength << ML_BITS);
    }

    memcpy(&dst[dstIdx], src, runLength);
    return dstIdx + runLength;
}

bool LZCodec::forward(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    if (input._array == output._array)
        return false;

    if (output._length  < getMaxEncodedLength(count))
        return false;

    const int hashLog = (count < MAX_DISTANCE) ? HASH_LOG_SMALL : HASH_LOG_BIG;
    const int hashShift = 32 - hashLog;
    const int matchLimit = count - LAST_LITERALS;
    const int mfLimit = count - MF_LIMIT;
    const int srcEnd = count;
    byte* src = &input._array[input._index];
    byte* dst = &output._array[ output._index];
    int srcIdx = 0;
    int dstIdx = 0;
    int anchor = 0;

    if (count > MIN_LENGTH) {
        if (_bufferSize < (1 << hashLog)) {
            _bufferSize = 1 << hashLog;
            _buffer = new int[_bufferSize];
        } 

        memset(_buffer, 0, sizeof(int) * (size_t(1) << hashLog));

        // First byte
        int* table = _buffer; // aliasing
        int h = (LittleEndian::readInt32(&src[srcIdx]) * LZ_HASH_SEED) >> hashShift;
        table[h] = srcIdx;
        srcIdx++;
        h = (LittleEndian::readInt32(&src[srcIdx]) * LZ_HASH_SEED) >> hashShift;

        while (true) {
            int fwdIdx = srcIdx;
            int step = 1;
            int searchMatchNb = SEARCH_MATCH_NB;
            int match;

            // Find a match
            do {
                srcIdx = fwdIdx;
                fwdIdx += step;

                if (fwdIdx > mfLimit) {
                    // Encode last literals
                    dstIdx += emitLastLiterals(&src[anchor], &dst[dstIdx], srcEnd - anchor);
                    input._index = srcEnd;
                    output._index = dstIdx;
                    return true;
                }

                step = searchMatchNb >> SKIP_STRENGTH;
                searchMatchNb++;
                match = table[h];
                table[h] = srcIdx;
                h = (LittleEndian::readInt32(&src[fwdIdx]) * LZ_HASH_SEED) >> hashShift;
            } while ((differentInts(src, match, srcIdx) == true) || (match <= srcIdx - MAX_DISTANCE));

            // Catch up
            while ((match > 0) && (srcIdx > anchor) && (src[match - 1] == src[srcIdx - 1])) {
                match--;
                srcIdx--;
            }

            // Encode literal length
            const int litLength = srcIdx - anchor;
            int token = dstIdx;
            dstIdx++;

            if (litLength >= RUN_MASK) {
                dst[token] = byte(RUN_MASK << ML_BITS);
                dstIdx += emitLength(&dst[dstIdx], litLength - RUN_MASK);
            }
            else {
                dst[token] = byte(litLength << ML_BITS);
            }

            // Copy literals
            customArrayCopy(&src[anchor], &dst[dstIdx], litLength);
            dstIdx += litLength;

            // Next match
            do {
                // Encode offset
                dst[dstIdx++] = byte(srcIdx - match);
                dst[dstIdx++] = byte((srcIdx - match) >> 8);

                // Encode match length
                srcIdx += MIN_MATCH;
                match += MIN_MATCH;
                anchor = srcIdx;

                while ((srcIdx < matchLimit) && (src[srcIdx] == src[match])) {
                    srcIdx++;
                    match++;
                }

                const int matchLength = srcIdx - anchor;

                // Encode match length
                if (matchLength >= ML_MASK) {
                    dst[token] |= byte(ML_MASK);
                    dstIdx += emitLength(&dst[dstIdx], matchLength - ML_MASK);
                }
                else {
                    dst[token] |= byte(matchLength);
                }

                anchor = srcIdx;

                if (srcIdx > mfLimit) {
                    // Encode last literals
                    dstIdx += emitLastLiterals(&src[anchor], &dst[dstIdx], srcEnd - anchor);
                    input._index = srcEnd;
                    output._index = dstIdx;
                    return true;
                }

                // Fill table
                h = (LittleEndian::readInt32(&src[srcIdx - 2]) * LZ_HASH_SEED) >> hashShift;
                table[h] = srcIdx - 2;

                // Test next position
                h = (LittleEndian::readInt32(&src[srcIdx]) * LZ_HASH_SEED) >> hashShift;
                match = table[h];
                table[h] = srcIdx;

                if ((differentInts(src, match, srcIdx) == true) || (match <= srcIdx - MAX_DISTANCE))
                    break;

                token = dstIdx;
                dstIdx++;
                dst[token] = byte(0);
            } while (true);

            // Prepare next loop
            srcIdx++;
            h = (LittleEndian::readInt32(&src[srcIdx]) * LZ_HASH_SEED) >> hashShift;
        }
    }

    // Encode last literals
    dstIdx += emitLastLiterals(&src[anchor], &dst[dstIdx], srcEnd - anchor);
    input._index = srcEnd;
    output._index = dstIdx;
    return true;
}

bool LZCodec::inverse(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    if (input._array == output._array)
        return false;

    byte* src = &input._array[input._index];
    byte* dst = &output._array[ output._index];
    uint8* usrc = (uint8*) src;
    const int srcEnd = count;
    const int dstEnd = output._length;
    const int srcEnd2 = srcEnd - COPY_LENGTH;
    const int dstEnd2 = dstEnd - COPY_LENGTH;
    int srcIdx = 0;
    int dstIdx = 0;

    while (true) {
        // Get literal length
        const int token = usrc[srcIdx++];
        int length = token >> ML_BITS;

        if (length == RUN_MASK) {
            uint8 len;

            while (((len = usrc[srcIdx++]) == uint8(0xFF)) && (srcIdx <= srcEnd))
                length += 0xFF;

            length += len;

            if (length > MAX_LENGTH) {
                stringstream ss;
                ss << "Invalid length decoded: " << length;
                throw invalid_argument(ss.str());
            }
        }

        // Copy literals
        if ((dstIdx + length > dstEnd2) || (srcIdx + length > srcEnd2)) {
            memcpy(&dst[dstIdx], &src[srcIdx], length);
            srcIdx += length;
            dstIdx += length;
            break;
        }

        customArrayCopy(&src[srcIdx], &dst[dstIdx], length);
        srcIdx += length;
        dstIdx += length;

        if ((dstIdx > dstEnd2) || (srcIdx > srcEnd2))
            break;

        // Get offset
        const int delta = usrc[srcIdx] | (usrc[srcIdx + 1] << 8);
        srcIdx += 2;

        if (dstIdx < delta)
            break;

        length = token & ML_MASK;

        // Get match length
        if (length == ML_MASK) {
            while (((src[srcIdx]) == byte(0xFF)) && (srcIdx < srcEnd)) {
                srcIdx++;
                length += 0xFF;
            }

            if (srcIdx < srcEnd)
                length += usrc[srcIdx++];

            if ((length > MAX_LENGTH) || (srcIdx == srcEnd)) {
                stringstream ss;
                ss << "Invalid length decoded: " << length;
                throw invalid_argument(ss.str());
            }
        }

        length += MIN_MATCH;
        int match = dstIdx - delta;
        const int cpy = dstIdx + length;

        // Copy repeated sequence
        if (cpy > dstEnd2) {
            byte* p1 = &dst[dstIdx];
            byte* p2 = &dst[match];

            for (int i = 0; i < length; i++)
                p1[i] = p2[i];
        }
        else {
            if (dstIdx >= match + 8) {
                do {
                    memcpy(&dst[dstIdx], &dst[match], 8);
                    match += 8;
                    dstIdx += 8;
                } while (dstIdx < cpy);
            }
            else {
                do {
                    byte* p1 = &dst[dstIdx];
                    byte* p2 = &dst[match];
                    p1[0] = p2[0];
                    p1[1] = p2[1];
                    p1[2] = p2[2];
                    p1[3] = p2[3];
                    p1[4] = p2[4];
                    p1[5] = p2[5];
                    p1[6] = p2[6];
                    p1[7] = p2[7];
                    match += 8;
                    dstIdx += 8;
                } while (dstIdx < cpy);
            }
        }

        // Correction
        dstIdx = cpy;
    }

    input._index = srcIdx;
    output._index = dstIdx;
    return srcIdx == srcEnd;
}