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

#include "SBRT.hpp"
#include <stdexcept>

using namespace kanzi;

SBRT::SBRT(int mode) :
	  _mask1((mode == MODE_TIMESTAMP) ? 0 : -1)
	, _mask2((mode == MODE_MTF) ? 0 : -1)
	, _shift((mode == MODE_RANK) ? 1 : 0)
{
    if ((mode != MODE_MTF) && (mode != MODE_RANK) && (mode != MODE_TIMESTAMP))
        throw invalid_argument("Invalid mode parameter");
}

SBRT::SBRT(int mode, Context&) :
    _mask1((mode == MODE_TIMESTAMP) ? 0 : -1)
    , _mask2((mode == MODE_MTF) ? 0 : -1)
    , _shift((mode == MODE_RANK) ? 1 : 0)
{
    if ((mode != MODE_MTF) && (mode != MODE_RANK) && (mode != MODE_TIMESTAMP))
        throw invalid_argument("Invalid mode parameter");
}

bool SBRT::forward(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    // Aliasing
    uint8* src = (uint8*) &input._array[input._index];
    byte* dst = &output._array[output._index];
    int p[256] = { 0 };
    int q[256] = { 0 };
    int s2r[256];
    int r2s[256];

    for (int i = 0; i < 256; i++) {
        s2r[i] = i;
        r2s[i] = i;
    }

    for (int i = 0; i < count; i++) {
        const uint8 c = src[i];
        int r = s2r[c];
        dst[i] = byte(r);
        const int qc = ((i & _mask1) + (p[c] & _mask2)) >> _shift;
        p[c] = i;
        q[c] = qc;

        // Move up symbol to correct rank
        while ((r > 0) && (q[r2s[r - 1]] <= qc)) {
            r2s[r] = r2s[r - 1];
            s2r[r2s[r]] = r;
            r--;
        }

        r2s[r] = c;
        s2r[c] = r;
    }

    input._index += count;
    output._index += count;
    return true;
}

bool SBRT::inverse(SliceArray<byte>& input, SliceArray<byte>& output, int count)
{
    if (count == 0)
        return true;

    if (!SliceArray<byte>::isValid(input))
        throw invalid_argument("Invalid input block");

    if (!SliceArray<byte>::isValid(output))
        throw invalid_argument("Invalid output block");

    // Aliasing
    uint8* src = (uint8*) &input._array[input._index];
    byte* dst = &output._array[output._index];
    int p[256] = { 0 };
    int q[256] = { 0 };
    int r2s[256];

    for (int i = 0; i < 256; i++)
        r2s[i] = i;

    for (int i = 0; i < count; i++) {
        uint8 r = src[i];
        const int c = r2s[r];
        dst[i] = byte(c);
        const int qc = ((i & _mask1) + (p[c] & _mask2)) >> _shift;
        p[c] = i;
        q[c] = qc;

        // Move up symbol to correct rank
        while ((r > 0) && (q[r2s[r - 1]] <= qc)) {
            r2s[r] = r2s[r - 1];
            r--;
        }

        r2s[r] = c;
    }

    input._index += count;
    output._index += count;
    return true;
}
