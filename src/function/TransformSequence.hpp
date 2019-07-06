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

#ifndef _TransformSequence_
#define _TransformSequence_

#include "../Function.hpp"

using namespace std;

namespace kanzi 
{

   // Encapsulates a sequence of transforms or functions in a function
   template <class T>
   class TransformSequence : public Function<T> {
   public:
       TransformSequence(Transform<T>* transforms[8], bool deallocate = true) THROW;

       ~TransformSequence();

       bool forward(SliceArray<T>& input, SliceArray<T>& output, int length);

       bool inverse(SliceArray<T>& input, SliceArray<T>& output, int length);

       // Required encoding output buffer size
       int getMaxEncodedLength(int srcLen) const;

       byte getSkipFlags() const { return _skipFlags; }

       void setSkipFlags(byte flags) { _skipFlags = flags; }

       int getNbFunctions() { return _length; }

   private:
       static const byte SKIP_MASK = byte(0xFF);

       Transform<T>* _transforms[8]; // transforms or functions
       bool _deallocate; // deallocate memory for transforms ?
       int _length; // number of transforms
       byte _skipFlags; // skip transforms
   };

   template <class T>
   TransformSequence<T>::TransformSequence(Transform<T>* transforms[8], bool deallocate) THROW
   {
       _deallocate = deallocate;
       _length = 8;
       _skipFlags = byte(0);

       for (int i = 7; i >= 0; i--) {
           _transforms[i] = transforms[i];

           if (_transforms[i] == nullptr)
               _length = i;
       }

       if (_length == 0)
           throw invalid_argument("At least one transform required");
   }

   template <class T>
   TransformSequence<T>::~TransformSequence()
   {
       if (_deallocate == true) {
           for (int i = 0; i < 8; i++) {
               if (_transforms[i] != nullptr)
                   delete _transforms[i];
           }
       }
   }

   template <class T>
   bool TransformSequence<T>::forward(SliceArray<T>& input, SliceArray<T>& output, int count) THROW
   {
       if (count == 0)
           return true;

       if (!SliceArray<byte>::isValid(input))
           throw invalid_argument("Invalid input block");

       if (!SliceArray<byte>::isValid(output))
           throw invalid_argument("Invalid output block");

       if ((count < 0) || (count + input._index > input._length))
           return false;

       const int blockSize = count;
       SliceArray<T>* sa[2] = { &input, &output };
       int saIdx = 0;
       const int requiredSize = getMaxEncodedLength(count);
       _skipFlags = byte(0);

       // Process transforms sequentially
       for (int i = 0; i < _length; i++) {
           SliceArray<T>* sa1 = sa[saIdx];
           saIdx ^= 1;
           SliceArray<T>* sa2 = sa[saIdx];

           // Check that the output buffer has enough room. If not, allocate a new one.
           if (sa2->_length < requiredSize) {
               delete[] sa2->_array;
               sa2->_array = new byte[requiredSize];
               sa2->_length = requiredSize;
           }

           const int savedIIdx = sa1->_index;
           const int savedOIdx = sa2->_index;
           Transform<T>* transform = _transforms[i];

           // Apply forward transform
           if (transform->forward(*sa1, *sa2, count) == false) {
               // Transform failed. Either it does not apply to this type
               // of data or a recoverable error occured => revert
               if (sa1->_array != sa2->_array)  
                   memmove(&sa2->_array[savedOIdx], &sa1->_array[savedIIdx], count);

               sa2->_index = savedOIdx + count;
               _skipFlags |= byte(1 << (7 - i));
           }

           count = sa2->_index - savedOIdx;
           sa1->_index = savedIIdx;
           sa2->_index = savedOIdx;
       }

       for (int i = _length; i < 8; i++)
           _skipFlags |= byte(1 << (7 - i));

       if (saIdx != 1)
           memmove(&sa[1]->_array[sa[1]->_index], &sa[0]->_array[sa[0]->_index], count);

       input._index += blockSize;
       output._index += count;
       return _skipFlags != SKIP_MASK;
   }

   template <class T>
   bool TransformSequence<T>::inverse(SliceArray<T>& input, SliceArray<T>& output, int length) THROW
   {
       if (length == 0)
           return true;

       if (!SliceArray<byte>::isValid(input))
           throw invalid_argument("Invalid input block");

       if (!SliceArray<byte>::isValid(output))
           throw invalid_argument("Invalid output block");

       if ((length < 0) || (length + input._index > input._length))
           return false;

       if (_skipFlags == SKIP_MASK) {
           if (&(input._array) != &(output._array))
               memmove(&output._array[output._index], &input._array[input._index], length);

           input._index += length;
           output._index += length;
           return true;
       }

       const int blockSize = length;
       const int count = output._length;
       bool res = true;
       SliceArray<T>* sa[2] = { &input, &output };
       int saIdx = 0;

       // Process transforms sequentially in reverse order
       for (int i = _length - 1; i >= 0; i--) {
           if ((_skipFlags & byte(1 << (7 - i))) != byte(0))
               continue;

           SliceArray<T>* sa1 = sa[saIdx];
           saIdx ^= 1;
           SliceArray<T>* sa2 = sa[saIdx];
           const int savedIIdx = sa1->_index;
           const int savedOIdx = sa2->_index;
           Transform<T>* transform = _transforms[i];

           // Apply inverse transform
           if (sa2->_length < output._length) {
              delete[] sa2->_array;
              sa2->_array = new byte[output._length];
           }

           sa1->_length = length;
           sa2->_length = count;

           res = transform->inverse(*sa1, *sa2, length);
           length = sa2->_index - savedOIdx;
           sa1->_index = savedIIdx;
           sa2->_index = savedOIdx;

           // All inverse transforms must succeed
           if (res == false)
               break;
       }

       if (saIdx != 1)
           memmove(&sa[1]->_array[sa[1]->_index], &sa[0]->_array[sa[0]->_index], length);

       input._index += blockSize;
       output._index += length;
       return res;
   }

   template <class T>
   int TransformSequence<T>::getMaxEncodedLength(int srcLength) const
   {
       int requiredSize = srcLength;

       for (int i = 0; i < _length; i++) {
           Function<T>* f = dynamic_cast<Function<T>*>(_transforms[i]);

           if (f != nullptr) {
               int reqSize = f->getMaxEncodedLength(srcLength);

               if (reqSize > requiredSize)
                   requiredSize = reqSize;
           }
       }

       return requiredSize;
   }
}
#endif
