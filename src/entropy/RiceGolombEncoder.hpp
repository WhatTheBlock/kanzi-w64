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

#ifndef _RiceGolombEncoder_
#define _RiceGolombEncoder_

#include "../EntropyEncoder.hpp"

namespace kanzi 
{

   class RiceGolombEncoder : public EntropyEncoder 
   {
   private:
       OutputBitStream& _bitstream;
       bool _signed;
       uint _logBase;
       int _base;

   public:
       RiceGolombEncoder(OutputBitStream& bitstream, uint logBase, bool sign=true) THROW;

       ~RiceGolombEncoder() { dispose(); }

       int encode(byte block[], uint blkptr, uint len);

       OutputBitStream& getBitStream() const { return _bitstream; };

       void encodeByte(byte val);

       void dispose() {}

       bool isSigned() const { return _signed; }
   };


   inline void RiceGolombEncoder::encodeByte(byte val)
   {
       if (val == byte(0))
       {
          _bitstream.writeBits(_base, _logBase+1);
          return;
       }

       int32 iVal = int32(val);
       iVal = (iVal + (iVal >> 31)) ^ (iVal >> 31); // abs(val2)

        // quotient is unary encoded, remainder is binary encoded
       int emit = _base | (iVal & (_base-1));
       int n = int(1 + (iVal >> _logBase)) + _logBase;

       if (_signed == true)
       {
          // Add 0 for positive and 1 for negative sign
          n++;
          emit = (emit << 1) | (uint32(val) >> 31);
       }

       _bitstream.writeBits(emit, n);
   }

}
#endif
