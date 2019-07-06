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

#ifndef _RLT_
#define _RLT_

#include "../Function.hpp"

namespace kanzi 
{

   // Implementation of an escaped RLE
   // Run length encoding:
   // RUN_LEN_ENCODE1 = 224 => RUN_LEN_ENCODE2 = 31*224 = 6944
   // 4    <= runLen < 224+4      -> 1 byte
   // 228  <= runLen < 6944+228   -> 2 bytes
   // 7172 <= runLen < 65535+7172 -> 3 bytes

   class RLT : public Function<byte> 
   {
   public:
       RLT() {}

       ~RLT() {}

       bool forward(SliceArray<byte>& pSrc, SliceArray<byte>& pDst, int length) THROW;

       bool inverse(SliceArray<byte>& pSrc, SliceArray<byte>& pDst, int length) THROW;

       int getMaxEncodedLength(int srcLen) const { return (srcLen <= 512) ? srcLen + 32 : srcLen; }

   private:
       static const int RUN_LEN_ENCODE1 = 224; // used to encode run length
       static const int RUN_LEN_ENCODE2 = (255 - RUN_LEN_ENCODE1) << 8; // used to encode run length
       static const int RUN_THRESHOLD = 3;
       static const int MAX_RUN = 0xFFFF + RUN_LEN_ENCODE2 + RUN_THRESHOLD - 1;
       static const int MAX_RUN4 = MAX_RUN - 4;

       int emitRunLength(byte* dst, int length, int run, byte escape, byte val);
   };

}
#endif
