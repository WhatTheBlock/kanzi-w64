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

#ifndef _DivSufSort_
#define _DivSufSort_

#include "../types.hpp"

#if __cplusplus >= 201103L
#include <utility>
#else
#include <algorithm>
#endif

using namespace std; // for C++17

namespace kanzi
{

    // DivSufSort is a fast two-stage suffix sorting algorithm by Yuta Mori.
    // The original C code is here: https://code.google.com/p/libdivsufsort/
    // See also https://code.google.com/p/libdivsufsort/source/browse/wiki/SACA_Benchmarks.wiki
    // for comparison of different suffix array construction algorithms.
    // It is used to implement the forward stage of the BWT in linear time.
    class StackElement
    {
        friend class DivSufSort;
        friend class Stack;

    private:
        int _a, _b, _c, _d, _e;

        StackElement();

        ~StackElement() {}
    };

    // A stack of pre-allocated elements
    class Stack
    {
        friend class DivSufSort;

    private:
        StackElement* _arr;
        int _index;
        int _length;

        Stack(int size);

        ~Stack();

        StackElement* get(int idx) const { return &_arr[idx]; }

        int size() const { return _index; }

        void push(int a, int b, int c, int d, int e);

        StackElement* pop() { return (_index == 0) ? nullptr : &_arr[--_index]; }
    };

    inline void Stack::push(int a, int b, int c, int d, int e)
    {
        StackElement* elt = &_arr[_index];
        elt->_a = a;
        elt->_b = b;
        elt->_c = c;
        elt->_d = d;
        elt->_e = e;
        _index++;
    }



    class TRBudget
    {
        friend class DivSufSort;

    private:
        int _chance;
        int _remain;
        int _incVal;
        int _count;

        TRBudget(int chance, int incval);

        ~TRBudget() {}

        bool check(int size);
    };


    inline bool TRBudget::check(int size)
    {
        if (size <= _remain) {
            _remain -= size;
            return true;
        }

        if (_chance == 0) {
            _count += size;
            return false;
        }

        _remain += (_incVal - size);
        _chance--;
        return true;
    }



    class DivSufSort
    {
    private:
        static const int SS_INSERTIONSORT_THRESHOLD = 8;
        static const int SS_BLOCKSIZE = 1024;
        static const int SS_MISORT_STACKSIZE = 16;
        static const int SS_SMERGE_STACKSIZE = 32;
        static const int TR_STACKSIZE = 64;
        static const int TR_INSERTIONSORT_THRESHOLD = 8;
        static const int SQQ_TABLE[];
        static const int LOG_TABLE[];

        int _length;
        int* _sa;
        uint8* _buffer;
        Stack* _ssStack;
        Stack* _trStack;
        Stack* _mergeStack;

        void constructSuffixArray(int32 bucketA[], int32 bucketB[], int n, int m);

        int constructBWT(int32 bucketA[], int32 bucketB[], int n, int m);

        int sortTypeBstar(int32 bucketA[], int32 bucketB[], int n);

        void ssSort(int pa, int first, int last, int buf, int bufSize,
            int depth, int n, bool lastSuffix);

        int ssCompare(int pa, int pb, int p2, int depth);

        int ssCompare(int p1, int p2, int depth);

        void ssInplaceMerge(int pa, int first, int middle, int last, int depth);

        void ssRotate(int first, int middle, int last);

        void ssBlockSwap(int a, int b, int n);

        static int getIndex(int a) { return (a >= 0) ? a : ~a; }

        void ssSwapMerge(int pa, int first, int middle, int last, int buf,
            int bufSize, int depth);

        void ssMergeForward(int pa, int first, int middle, int last, int buf,
            int depth);

        void ssMergeBackward(int pa, int first, int middle, int last, int buf,
            int depth);

        void ssInsertionSort(int pa, int first, int last, int depth);

        int ssIsqrt(int x);

        void ssMultiKeyIntroSort(const int pa, int first, int last, int depth);

        int ssPivot(int td, int pa, int first, int last);

        int ssMedian5(const int idx, int pa, int v1, int v2, int v3, int v4, int v5);

        int ssMedian3(int idx, int pa, int v1, int v2, int v3);

        int ssPartition(int pa, int first, int last, int depth);

        void ssHeapSort(int idx, int pa, int saIdx, int size);

        void ssFixDown(int idx, int pa, int saIdx, int i, int size);

        int ssIlg(int n);

        void trSort(int n, int depth);

        uint64 trPartition(int isad, int first, int middle, int last, int v);

        void trIntroSort(int isa, int isad, int first, int last, TRBudget& budget);

        int trPivot(int arr[], int isad, int first, int last);

        int trMedian5(int arr[], int isad, int v1, int v2, int v3, int v4, int v5);

        int trMedian3(int arr[], int isad, int v1, int v2, int v3);

        void trHeapSort(int isad, int saIdx, int size);

        void trFixDown(int isad, int saIdx, int i, int size);

        void trInsertionSort(int isad, int first, int last);

        void trPartialCopy(int isa, int first, int a, int b, int last, int depth);

        void trCopy(int isa, int first, int a, int b, int last, int depth);

        void reset();

        int trIlg(int n);

    public:
        DivSufSort();

        ~DivSufSort();

        void computeSuffixArray(byte input[], int sa[], int start, int length);

        int computeBWT(byte input[], int sa[], int start, int length);
    };


    inline int DivSufSort::ssIlg(int n)
    {
        return ((n & 0xFF00) != 0) ? 8 + LOG_TABLE[(n >> 8) & 0xFF]
            : LOG_TABLE[n & 0xFF];
    }


    inline int DivSufSort::trIlg(int n)
    {
        return ((n & 0xFFFF0000) != 0) ? (((n & 0xFF000000) != 0) ? 24 + LOG_TABLE[(n >> 24) & 0xFF]
            : 16 + LOG_TABLE[(n >> 16) & 0xFF])
            : (((n & 0x0000FF00) != 0) ? 8 + LOG_TABLE[(n >> 8) & 0xFF]
                : LOG_TABLE[n & 0xFF]);
    }



    inline int DivSufSort::trMedian5(int _sa[], int isad, int v1, int v2, int v3, int v4, int v5)
    {
        if (_sa[isad + _sa[v2]] > _sa[isad + _sa[v3]]) {
            swap(v2, v3);
        }

        if (_sa[isad + _sa[v4]] > _sa[isad + _sa[v5]]) {
            const int t = v4;
            v4 = v5;
            v5 = t;
        }

        if (_sa[isad + _sa[v2]] > _sa[isad + _sa[v4]]) {
            swap(v2, v4);
            swap(v3, v5);
        }

        if (_sa[isad + _sa[v1]] > _sa[isad + _sa[v3]]) {
            swap(v1, v3);
        }

        if (_sa[isad + _sa[v1]] > _sa[isad + _sa[v4]]) {
            swap(v1, v4);
            swap(v3, v5);
        }

        if (_sa[isad + _sa[v3]] > _sa[isad + _sa[v4]])
            return v4;

        return v3;
    }


    inline int DivSufSort::trMedian3(int _sa[], int isad, int v1, int v2, int v3)
    {
        if (_sa[isad + _sa[v1]] > _sa[isad + _sa[v2]]) {
            swap(v1, v2);
        }

        if (_sa[isad + _sa[v2]] > _sa[isad + _sa[v3]]) {
            if (_sa[isad + _sa[v1]] > _sa[isad + _sa[v3]])
                return v1;

            return v3;
        }

        return v2;
    }


    inline int DivSufSort::ssIsqrt(int x)
    {
        if (x >= (SS_BLOCKSIZE * SS_BLOCKSIZE))
            return SS_BLOCKSIZE;

        const int e = ((x & 0xFFFF0000) != 0) ? (((x & 0xFF000000) != 0) ? 24 + LOG_TABLE[(x >> 24) & 0xFF]
            : 16 + LOG_TABLE[(x >> 16) & 0xFF])
            : (((x & 0x0000FF00) != 0) ? 8 + LOG_TABLE[(x >> 8) & 0xFF]
                : LOG_TABLE[x & 0xFF]);

        if (e < 8)
            return SQQ_TABLE[x] >> 4;

        int y;

        if (e >= 16) {
            y = SQQ_TABLE[x >> ((e - 6) - (e & 1))] << ((e >> 1) - 7);

            if (e >= 24) {
                y = (y + 1 + x / y) >> 1;
            }

            y = (y + 1 + x / y) >> 1;
        }
        else {
            y = (SQQ_TABLE[x >> ((e - 6) - (e & 1))] >> (7 - (e >> 1))) + 1;
        }

        return (x < y* y) ? y - 1 : y;
    }

}
#endif