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

#ifndef _BWT_
#define _BWT_

#include "../Transform.hpp"
#include "../concurrent.hpp"
#include "DivSufSort.hpp"

using namespace std;

namespace kanzi {
// The Burrows-Wheeler Transform is a reversible transform based on
// permutation of the data in the original message to reduce the entropy.

// The initial text can be found here:
// Burrows M and Wheeler D, [A block sorting lossless data compression algorithm]
// Technical Report 124, Digital Equipment Corporation, 1994

// See also Peter Fenwick, [Block sorting text compression - final report]
// Technical Report 130, 1996

// This implementation replaces the 'slow' sorting of permutation strings
// with the construction of a suffix array (faster but more complex).
//
// E.G.    0123456789A
// Source: mississippi\0
// Suffixes:    rank  sorted
// mississippi\0  0  -> 4             i\0
//  ississippi\0  1  -> 3          ippi\0
//   ssissippi\0  2  -> 10      issippi\0
//    sissippi\0  3  -> 8    ississippi\0
//     issippi\0  4  -> 2   mississippi\0
//      ssippi\0  5  -> 9            pi\0
//       sippi\0  6  -> 7           ppi\0
//        ippi\0  7  -> 1         sippi\0
//         ppi\0  8  -> 6      sissippi\0
//          pi\0  9  -> 5        ssippi\0
//           i\0  10 -> 0     ssissippi\0
// Suffix array SA : 10 7 4 1 0 9 8 6 3 5 2
// BWT[i] = input[SA[i]-1] => BWT(input) = ipssmpissii (+ primary index 5)
// The suffix array and permutation vector are equal when the input is 0 terminated
// The insertion of a guard is done internally and is entirely transparent.
//
// This implementation extends the canonical algorithm to use up to MAX_CHUNKS primary
// indexes (based on input block size). Each primary index corresponds to a data chunk.
// Chunks may be inverted concurrently.
   template <class T>
   class InverseBigChunkTask : public Task<T> {
   private:
       uint* _data;
       uint* _buckets;
       uint16* _fastBits;
       int* _primaryIndexes;
       byte* _dst;
       int _total;
       int _start;
       int _ckSize;
       int _firstChunk;
       int _lastChunk;

   public:
       InverseBigChunkTask(uint* buf, uint* buckets, uint16* fastBits, byte* output,
           int* primaryIndexes, int total, int start, int ckSize, int firstChunk, int lastChunk);
       ~InverseBigChunkTask() {}

       T run() THROW;
   };

   class BWT : public Transform<byte> {

   private:
       static const int MAX_BLOCK_SIZE = 1024 * 1024 * 1024; // 1024 MB
       static const int BWT_MAX_CHUNKS = 8;
       static const int NB_FASTBITS = 17;

       uint* _buffer; 
       int* _sa; 
       int _bufferSize;
       int _primaryIndexes[8];
       DivSufSort _saAlgo;
       int _jobs;

       bool inverseBigBlock(SliceArray<byte>& input, SliceArray<byte>& output, int count);

       bool inverseSmallBlock(SliceArray<byte>& input, SliceArray<byte>& output, int count);

   public:
       static const int MASK_FASTBITS = (1 << NB_FASTBITS) - 1;

       BWT(int jobs = 1);

       virtual ~BWT();

       bool forward(SliceArray<byte>& input, SliceArray<byte>& output, int length) THROW;

       bool inverse(SliceArray<byte>& input, SliceArray<byte>& output, int length) THROW;

       int getPrimaryIndex(int n) const { return _primaryIndexes[n]; }

       bool setPrimaryIndex(int n, int primaryIndex);

       static int maxBlockSize() { return MAX_BLOCK_SIZE; }

       static int getBWTChunks(int size);
   };
}
#endif
