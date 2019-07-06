
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

#include <map>
#include <vector>
#include "BWT.hpp"
#include "../Global.hpp"

#ifdef CONCURRENCY_ENABLED
#include <future>
#endif

using namespace kanzi;

BWT::BWT(int jobs) THROW
{
    _buffer = nullptr;
    _sa = nullptr;
    _bufferSize = 0;

#ifndef CONCURRENCY_ENABLED
    if (jobs > 1)
        throw invalid_argument("The number of jobs is limited to 1 in this version");
#endif

    _jobs = jobs;
    memset(_primaryIndexes, 0, sizeof(int) * 8);
}

BWT::~BWT()
{
    if (_buffer != nullptr)
        delete[] _buffer;

    if (_sa != nullptr)
        delete[] _sa;
}

bool BWT::setPrimaryIndex(int n, int primaryIndex)
{
    if ((primaryIndex < 0) || (n < 0) || (n >= 8))
        return false;

    _primaryIndexes[n] = primaryIndex;
    return true;
}

bool BWT::forward(SliceArray<byte>& input, SliceArray<byte>& output, int count) THROW
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    if (count > maxBlockSize()) {
        // Not a recoverable error: instead of silently fail the transform,
        // issue a fatal error.
        stringstream ss;
        ss << "The max BWT block size is " << maxBlockSize() << ", got " << count;
        throw invalid_argument(ss.str());
    }

    if (count < 2) {
        if (count == 1)
            output._array[output._index++] = input._array[input._index++];

        return true;
    }

    byte* src = &input._array[input._index];
    byte* dst = &output._array[output._index];

    // Lazy dynamic memory allocation
    if ((_sa == nullptr) || (_bufferSize < count)) {
        if (_sa != nullptr)
            delete[] _sa;

        _bufferSize = count;
        _sa = new int[_bufferSize];
    }

    int* sa = _sa;
    _saAlgo.computeSuffixArray(src, sa, 0, count);
    const int chunks = getBWTChunks(count);
    bool res = true;

    if (chunks == 1) {
        dst[0] = src[count - 1];
        int n = 0;

        for (; n < count; n++) {
            if (sa[n] == 0)
                break;

            dst[n + 1] = src[sa[n] - 1];
        }

        n++;
        res &= setPrimaryIndex(0, n);

        for (; n < count; n++)
            dst[n] = src[sa[n] - 1];
    }
    else {
        const int st = count / chunks;
        const int step = (chunks * st == count) ? st : st + 1;
        dst[0] = src[count - 1];
        int idx = 0;

        for (int i = 0; i < count; i++) {
            if ((sa[i] % step) != 0)
                continue;

            res &= setPrimaryIndex(sa[i] / step, i + 1);
            idx++;

            if (idx == chunks)
                break;
        }

        const int pIdx0 = getPrimaryIndex(0);

        for (int i = 0; i < pIdx0 - 1; i++)
            dst[i + 1] = src[sa[i] - 1];

        for (int i = pIdx0; i < count; i++)
            dst[i] = src[sa[i] - 1];
    }

    input._index += count;
    output._index += count;
    return res;
}

bool BWT::inverse(SliceArray<byte>& input, SliceArray<byte>& output, int count) THROW
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    if (count > maxBlockSize()) {
        // Not a recoverable error: instead of silently fail the transform,
        // issue a fatal error.
        stringstream ss;
        ss << "The max BWT block size is " << maxBlockSize() << ", got " << count;
        throw invalid_argument(ss.str());
    }

    if (count < 2) {
        if (count == 1)
            output._array[output._index++] = input._array[input._index++];

        return true;
    }

    // Find the fastest way to implement inverse based on block size
    if (count < 4 * 1024 * 1024)
        return inverseSmallBlock(input, output, count);

    return inverseBigBlock(input, output, count);
}

// When count < 4M, mergeTPSI algo, always one chunk
bool BWT::inverseSmallBlock(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    // Lazy dynamic memory allocation
    if ((_buffer == nullptr) || (_bufferSize < count)) {
        if (_buffer != nullptr)
            delete[] _buffer;

        _bufferSize = count;
        _buffer = new uint[_bufferSize];
    }

    uint8* src = (uint8*)&input._array[input._index];
    byte* dst = &output._array[output._index];

    const int pIdx = getPrimaryIndex(0);

    if ((pIdx < 0) || (pIdx > count))
        return false;

    // Build array of packed index + value (assumes block size < 2^24)
    uint buckets[256] = { 0 };
    uint* data = _buffer;
    Global::computeHistogram(&input._array[input._index], count, buckets, true);

    for (int i = 0, sum = 0; i < 256; i++) {
        const int tmp = buckets[i];
        buckets[i] = sum;
        sum += tmp;
    }

    for (int i = 0; i < pIdx; i++) {
        const uint8 val = src[i];
        data[buckets[val]] = ((i - 1) << 8) | val;
        buckets[val]++;
    }

    for (int i = pIdx; i < count; i++) {
        const uint8 val = src[i];
        data[buckets[val]] = (i << 8) | val;
        buckets[val]++;
    }

    // Build inverse
    for (int i = 0, t = pIdx - 1; i < count; i++) {
        const uint ptr = data[t];
        dst[i] = byte(ptr);
        t = ptr >> 8;
    }

    input._index += count;
    output._index += count;
    return true;
}

// When count >= 4M, biPSIv2 algo, possibly several chunks
bool BWT::inverseBigBlock(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    // Lazy dynamic memory allocations
    if ((_buffer == nullptr) || (_bufferSize < count + 1)) {
        if (_buffer != nullptr)
            delete[] _buffer;

        _bufferSize = count + 1;
        _buffer = new uint[_bufferSize];
    }

    uint8* src = (uint8*)&input._array[input._index];
    byte* dst = &output._array[output._index];
    const int pIdx = getPrimaryIndex(0);

    if ((pIdx < 0) || (pIdx > count))
        return false;

    uint* buckets = new uint[65536];
    memset(&buckets[0], 0, 65536 * sizeof(uint));
    uint freqs[256];
    Global::computeHistogram(&input._array[input._index], count, freqs, true);

    for (int sum = 1, c = 0; c < 256; c++) {
        const int f = sum;
        sum += int(freqs[c]);
        freqs[c] = f;

        if (f != sum) {
            uint* ptr = &buckets[c << 8];
            const int hi = (sum < pIdx) ? sum : pIdx;

            for (int i = f; i < hi; i++)
                ptr[src[i]]++;

            const int lo = (f - 1 > pIdx) ? f - 1 : pIdx;

            for (int i = lo; i < sum - 1; i++)
                ptr[src[i]]++;
        }
    }

    const int lastc = src[0];
    uint16* fastBits = new uint16[MASK_FASTBITS + 1];
    memset(&fastBits[0], 0, (MASK_FASTBITS + 1) * sizeof(uint16));
    int shift = 0;

    while ((count >> shift) > MASK_FASTBITS)
        shift++;

    for (int v = 0, sum = 1, c = 0; c < 256; c++) {
        if (c == lastc)
            sum++;

        uint* ptr = &buckets[c];

        for (int d = 0; d < 256; d++) {
            const int s = sum;
            sum += ptr[d << 8];
            ptr[d << 8] = s;

            if (s != sum) {
                for (; v <= ((sum - 1) >> shift); v++)
                    fastBits[v] = uint16((c << 8) | d);
            }
        }
    }

    uint* data = &_buffer[0];

    for (int i = 0; i < pIdx; i++) {
        const uint8 c = src[i];
        const int p = freqs[c];
        freqs[c]++;

        if (p < pIdx)
            data[buckets[(c << 8) | src[p]]++] = i;
        else if (p > pIdx)
            data[buckets[(c << 8) | src[p - 1]]++] = i;
    }

    for (int i = pIdx; i < count; i++) {
        const uint8 c = src[i];
        const int p = freqs[c];
        freqs[c]++;

        if (p < pIdx)
            data[buckets[(c << 8) | src[p]]++] = i + 1;
        else if (p > pIdx)
            data[buckets[(c << 8) | src[p - 1]]++] = i + 1;
    }

    for (int c = 0; c < 256; c++) {
        const int c256 = c << 8;
        
        for (int d = 0; d < c; d++) {
            const int tmp = buckets[(d << 8) | c];
            buckets[(d << 8) | c] = buckets[c256 | d];
            buckets[c256 | d] = tmp;
        }
    }

    const int chunks = getBWTChunks(count);

    // Build inverse
    if (chunks == 1) {
        uint p = pIdx;

        for (int i = 1; i < count; i += 2) {
            uint16 c = fastBits[p >> shift];

            while (buckets[c] <= p)
                c++;

            dst[i - 1] = byte(c >> 8);
            dst[i] = byte(c);
            p = data[p];
        }
    }
#ifdef CONCURRENCY_ENABLED
    else {
        // Several chunks may be decoded concurrently (depending on the availability
        // of jobs per block).
        const int st = count / chunks;
        const int ckSize = (chunks * st == count) ? st : st + 1;
        const int nbTasks = (_jobs < chunks) ? _jobs : chunks;
        int* jobsPerTask = new int[nbTasks];
        Global::computeJobsPerTask(jobsPerTask, chunks, nbTasks);
        vector<future<int> > futures;
        vector<InverseBigChunkTask<int>*> tasks;

        // Create one task per job
        for (int j = 0, c = 0; j < nbTasks; j++) {
            // Each task decodes jobsPerTask[j] chunks
            const int start = c * ckSize;

            InverseBigChunkTask<int>* task = new InverseBigChunkTask<int>(data, buckets, fastBits, dst, _primaryIndexes,
                count, start, ckSize, c, c + jobsPerTask[j]);
            tasks.push_back(task);
            futures.push_back(async(launch::async, &InverseBigChunkTask<int>::run, task));
            c += jobsPerTask[j];
        }

        // Wait for completion of all concurrent tasks
        for (int j = 0; j < nbTasks; j++) {
            futures[j].get();
        }

        // Cleanup
        for (vector<InverseBigChunkTask<int>*>::iterator it = tasks.begin(); it != tasks.end(); it++)
            delete *it;

        tasks.clear();
        delete[] jobsPerTask;
    }
#endif

    dst[count - 1] = byte(lastc);
    delete[] fastBits;
    delete[] buckets;
    input._index += count;
    output._index += count;
    return true;
}

template <class T>
InverseBigChunkTask<T>::InverseBigChunkTask(uint* buf, uint* buckets, uint16* fastBits, byte* output,
    int* primaryIndexes, int total, int start, int ckSize, int firstChunk, int lastChunk)
{
    _data = buf;
    _fastBits = fastBits;
    _buckets = buckets;
    _primaryIndexes = primaryIndexes;
    _dst = output;
    _total = total;
    _start = start;
    _ckSize = ckSize;
    _firstChunk = firstChunk;
    _lastChunk = lastChunk;
}

template <class T>
T InverseBigChunkTask<T>::run() THROW
{
	int shift = 0;

	while ((_total >> shift) > BWT::MASK_FASTBITS)
		shift++;

	for (int c = _firstChunk; c < _lastChunk; c++) {
		int end = (_start + _ckSize) > _total - 1 ? _total - 1 : _start + _ckSize;
		uint p = _primaryIndexes[c];

        for (int i = _start+1; i <= end; i += 2) {
            uint16 s = _fastBits[p >> shift];

            while (_buckets[s] <= p)
                s++;

            _dst[i - 1] = byte(s >> 8);
            _dst[i] = byte(s);
            p = _data[p];
        }

        _start = end;
	}

	return T(0);
}

int BWT::getBWTChunks(int size)
{
    if (size < 4 * 1024 * 1024)
        return 1;

    const int res = (size + (1 << 21)) >> 22;
    return (res > BWT_MAX_CHUNKS) ? BWT_MAX_CHUNKS : res;
}
