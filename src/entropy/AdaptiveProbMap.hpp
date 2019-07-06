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

#ifndef _AdaptiveProbMap_
#define _AdaptiveProbMap_

#include "../Global.hpp"

// APM maps a probability and a context into a new probability
// that the next bit will be 1. After each guess, it updates
// its state to improve future guesses.

namespace kanzi {
   template <int RATE>
   class LinearAdaptiveProbMap {
   public:
       LinearAdaptiveProbMap<RATE>(int n);

       ~LinearAdaptiveProbMap<RATE>() { delete[] _data; }

       int get(int bit, int pr, int ctx);

   private:
       int _index; // last p, context
       uint16* _data; // [NbCtx][33]:  p, context -> p
   };

   template <int RATE>
   inline LinearAdaptiveProbMap<RATE>::LinearAdaptiveProbMap(int n)
   {
       _data = new uint16[65 * n];
       _index = 0;

       for (int j = 0; j <= 64; j++) {
           _data[j] = uint16(j << 6) << 4;
       }

       for (int i = 1; i < n; i++) {
           memcpy(&_data[i * 65], &_data[0], 65 * sizeof(uint16));
       }
   }

   // Return improved prediction given current bit, prediction and context
   template <int RATE>
   inline int LinearAdaptiveProbMap<RATE>::get(int bit, int pr, int ctx)
   {
       // Update probability based on error and learning rate
       const int g = (-bit & 65528) + (bit << RATE);
       _data[_index] += uint16((g - int(_data[_index])) >> RATE);
       _data[_index + 1] += uint16((g - int(_data[_index + 1])) >> RATE);

       // Find index: 65*ctx + quantized prediction in [0..64]
       _index = (pr >> 6) + 65 * ctx;

       // Return interpolated probabibility
       const uint16 w = uint16(pr & 127);
       return int(_data[_index] * (128 - w) + _data[_index + 1] * w) >> 11;
   }

   template <int RATE>
   class LogisticAdaptiveProbMap {
   public:
       LogisticAdaptiveProbMap<RATE>(int n);

       ~LogisticAdaptiveProbMap<RATE>() { delete[] _data; }

       int get(int bit, int pr, int ctx);

   private:
       int _index; // last p, context
       uint16* _data; // [NbCtx][33]:  p, context -> p
   };

   template <int RATE>
   inline LogisticAdaptiveProbMap<RATE>::LogisticAdaptiveProbMap(int n)
   {
       _data = new uint16[33 * n];
       _index = 0;

       for (int j = 0; j <= 32; j++) {
           _data[j] = uint16(Global::squash((j - 16) << 7)) << 4;
       }

       for (int i = 1; i < n; i++) {
           memcpy(&_data[i * 33], &_data[0], 33 * sizeof(uint16));
       }
   }

   // Return improved prediction given current bit, prediction and context
   template <int RATE>
   inline int LogisticAdaptiveProbMap<RATE>::get(int bit, int pr, int ctx)
   {
       // Update probability based on error and learning rate
       const int g = (-bit & 65528) + (bit << RATE);
       _data[_index] += uint16((g - int(_data[_index])) >> RATE);
       _data[_index + 1] += uint16((g - int(_data[_index + 1])) >> RATE);
       pr = Global::STRETCH[pr];

       // Find index: 33*ctx + quantized prediction in [0..32]
       _index = ((pr + 2048) >> 7) + 33 * ctx;

       // Return interpolated probabibility
       const uint16 w = uint16(pr & 127);
       return int(_data[_index] * (128 - w) + _data[_index + 1] * w) >> 11;
   }

   template <int RATE>
   class FastLogisticAdaptiveProbMap {
   public:
       FastLogisticAdaptiveProbMap<RATE>(int n);

       ~FastLogisticAdaptiveProbMap<RATE>() { delete[] _data; }

       int get(int bit, int pr, int ctx);

   private:
       uint16* _p; // last p
       uint16* _data; // [NbCtx][32]:  p, context -> p
   };

   template <int RATE>
   inline FastLogisticAdaptiveProbMap<RATE>::FastLogisticAdaptiveProbMap(int n)
   {
       _data = new uint16[32 * n];
       _p = &_data[0];

       for (int j = 0; j < 32; j++) {
           _data[j] = uint16(Global::squash((j - 16) << 7)) << 4;
       }

       for (int i = 1; i < n; i++) {
           memcpy(&_data[i * 32], &_data[0], 32 * sizeof(uint16));
       }
   }

   // Return improved prediction given current bit, prediction and context
   template <int RATE>
   inline int FastLogisticAdaptiveProbMap<RATE>::get(int bit, int pr, int ctx)
   {
       // Update probability based on error and learning rate
       const int g = (-bit & 65528) + (bit << RATE);
       _p[0] += ((g - int(_p[0])) >> RATE);

       // Find index: 32*ctx + quantized prediction in [0..32[
       _p = &_data[((Global::STRETCH[pr] + 2048) >> 7) + 32 * ctx];
       return int(_p[0]) >> 4;
   }
}
#endif
