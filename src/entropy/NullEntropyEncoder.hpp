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

#ifndef _NullEntropyEncoder_
#define _NullEntropyEncoder_

#include "../Memory.hpp"

namespace kanzi {

   // Null entropy encoder
   // Pass through that writes the data directly to the bitstream
   class NullEntropyEncoder : public EntropyEncoder {
   private:
       OutputBitStream& _bitstream;

   public:
       NullEntropyEncoder(OutputBitStream& bitstream);

       ~NullEntropyEncoder() { dispose(); }

       int encode(byte arr[], uint blkptr, uint len);

       void encodeByte(byte val);

       OutputBitStream& getBitStream() const { return _bitstream; };

       void dispose() {}
   };

   inline NullEntropyEncoder::NullEntropyEncoder(OutputBitStream& bitstream)
       : _bitstream(bitstream)
   {
   }

   inline int NullEntropyEncoder::encode(byte block[], uint blkptr, uint count)
   {
      int res = 0;

      while (count > 0) {
	      const int ckSize = (count < 1<<23) ? count : 1<<23;
	      res += (_bitstream.writeBits(&block[blkptr], 8 * ckSize) >> 3);
	      blkptr += ckSize;
	      count -= ckSize;
      }

      return res;
   }

   inline void NullEntropyEncoder::encodeByte(byte val) {
      _bitstream.writeBits(uint64(val), 8);
   }
}
#endif
