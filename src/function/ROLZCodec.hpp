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

#ifndef _ROLZCodec_
#define _ROLZCodec_

#include <map>
#include "../Function.hpp"
#include "../Memory.hpp"
#include "../Predictor.hpp"
#include "../util.hpp"


using namespace std;

// Implementation of a Reduced Offset Lempel Ziv transform
// More information about ROLZ at http://ezcodesample.com/rolz/rolz_article.html

namespace kanzi {
	class ROLZPredictor : public Predictor {
	private:
		uint16* _probs;
		uint _logSize;
		int32 _size;
		int32 _c1;
		int32 _ctx;

	public:
		ROLZPredictor(uint logMaxSymbolSize);

		~ROLZPredictor()
		{
			delete[] _probs;
		};

		void reset();

		void update(int bit);

		int get();

		void setContext(byte ctx) { _ctx = uint8(ctx) << _logSize; }
	};

	class ROLZEncoder {
	private:
		static const uint64 TOP = 0x00FFFFFFFFFFFFFF;
		static const uint64 MASK_24_56 = 0x00FFFFFFFF000000;
		static const uint64 MASK_0_24 = 0x0000000000FFFFFF;
		static const uint64 MASK_0_32 = 0x00000000FFFFFFFF;

		Predictor* _predictors[2];
		Predictor* _predictor;
		byte* _buf;
		int& _idx;
		uint64 _low;
		uint64 _high;

	public:
		ROLZEncoder(Predictor* predictors[2], byte buf[], int& idx);

		~ROLZEncoder() {}

		void encodeByte(byte val);

		void encodeBit(int bit);

		void dispose();

		void setContext(int n) { _predictor = _predictors[n]; }
	};

	class ROLZDecoder {
	private:
		static const uint64 TOP = 0x00FFFFFFFFFFFFFF;
		static const uint64 MASK_0_56 = 0x00FFFFFFFFFFFFFF;
		static const uint64 MASK_0_32 = 0x00000000FFFFFFFF;

		Predictor* _predictors[2];
		Predictor* _predictor;
		byte* _buf;
		int& _idx;
		uint64 _low;
		uint64 _high;
		uint64 _current;

	public:
		ROLZDecoder(Predictor* predictors[2], byte buf[], int& idx);

		~ROLZDecoder() {}

		byte decodeByte();

		int decodeBit();

		void dispose() {}

		void setContext(int n) { _predictor = _predictors[n]; }
	};

	// Use ANS to encode/decode literals and matches
	class ROLZCodec1 : public Function<byte> {
	public:
		ROLZCodec1(uint logPosChecks) THROW;

		~ROLZCodec1() { delete[] _matches; }

		bool forward(SliceArray<byte>& src, SliceArray<byte>& dst, int length) THROW;

		bool inverse(SliceArray<byte>& src, SliceArray<byte>& dst, int length) THROW;

		// Required encoding output buffer size
		int getMaxEncodedLength(int srcLen) const;

	private:
		int32* _matches;
		int32 _counters[65536];
		int _logPosChecks;
		int _maskChecks;
		int _posChecks;

		int findMatch(const byte buf[], const int pos, const int end);	

		void emitLengths(SliceArray<byte>& lenBuf, int litLen, int mLen);

		void readLengths(SliceArray<byte>& lenBuf, int& litLen, int& mLen);

		int emitLiterals(SliceArray<byte>& litBuf, byte dst[], int dstIdx, int startIdx, int litLen);
	};

	// Use CM (ROLZEncoder/ROLZDecoder) to encode/decode literals and matches
	// Code loosely based on 'balz' by Ilya Muravyov
	class ROLZCodec2 : public Function<byte> {
	public:
		ROLZCodec2(uint logPosChecks) THROW;

		~ROLZCodec2() { delete[] _matches; }

		bool forward(SliceArray<byte>& src, SliceArray<byte>& dst, int length) THROW;

		bool inverse(SliceArray<byte>& src, SliceArray<byte>& dst, int length) THROW;

		// Required encoding output buffer size
		int getMaxEncodedLength(int srcLen) const;

	private:
		static const int LITERAL_FLAG = 0;
		static const int MATCH_FLAG = 1;

		int32* _matches;
		int32 _counters[65536];
		int _logPosChecks;
		int _maskChecks;
		int _posChecks;
		ROLZPredictor _litPredictor;
		ROLZPredictor _matchPredictor;

		int findMatch(const byte buf[], const int pos, const int end);
	};

	class ROLZCodec : public Function<byte> {
		friend class ROLZCodec1;
		friend class ROLZCodec2;

	public:
		ROLZCodec(uint logPosChecks = LOG_POS_CHECKS2) THROW;

		ROLZCodec(map<string, string>& ctx) THROW;

		virtual ~ROLZCodec()
		{
		   delete _delegate;
		}

		bool forward(SliceArray<byte>& src, SliceArray<byte>& dst, int length) THROW;

		bool inverse(SliceArray<byte>& src, SliceArray<byte>& dst, int length) THROW;

		// Required encoding output buffer size
		int getMaxEncodedLength(int srcLen) const
		{
		   return _delegate->getMaxEncodedLength(srcLen);
		}

	private:
		static const int HASH_SIZE = 1 << 16;
		static const int MIN_MATCH = 3;
		static const int MAX_MATCH = MIN_MATCH + 255;
		static const int LOG_POS_CHECKS1 = 4;
		static const int LOG_POS_CHECKS2 = 5;
		static const int CHUNK_SIZE = 1 << 26; // 64 MB
		static const int32 HASH = 200002979;
		static const int32 HASH_MASK = ~(CHUNK_SIZE - 1);
		static const int MAX_BLOCK_SIZE = 1 << 27; // 128 MB

		Function<byte>* _delegate;

		static uint16 getKey(const byte* p)
		{
			return uint16(LittleEndian::readInt16(p));
		}

		static int32 hash(const byte* p)
		{
			return ((LittleEndian::readInt32(p) & 0x00FFFFFF) * HASH) & HASH_MASK;
		}

		static int emitCopy(byte dst[], int dstIdx, int ref, int matchLen);
   };


   inline void ROLZPredictor::update(int bit)
   {
	   _probs[_ctx + _c1] -= (((_probs[_ctx + _c1] - uint16(-bit)) >> 5) + bit);
	   _c1 = (_c1 << 1) + bit;

	   if (_c1 >= _size)
	      _c1 = 1;
   }
   

   inline int ROLZPredictor::get()
   {
	   return int(_probs[_ctx + _c1]) >> 4;
   }


   inline int ROLZCodec::emitCopy(byte dst[], int dstIdx, int ref, int matchLen)
   {
	   dst[dstIdx] = dst[ref];
	   dst[dstIdx + 1] = dst[ref + 1];
	   dst[dstIdx + 2] = dst[ref + 2];
	   dstIdx += 3;
	   ref += 3;

	   while (matchLen >= 8) {
	      dst[dstIdx] = dst[ref];
	      dst[dstIdx + 1] = dst[ref + 1];
	      dst[dstIdx + 2] = dst[ref + 2];
 	      dst[dstIdx + 3] = dst[ref + 3];
 	      dst[dstIdx + 4] = dst[ref + 4];
 	      dst[dstIdx + 5] = dst[ref + 5];
 	      dst[dstIdx + 6] = dst[ref + 6];
 	      dst[dstIdx + 7] = dst[ref + 7];
	      dstIdx += 8;
	      ref += 8;
	      matchLen -= 8;
	   }

	   while (matchLen != 0) {
	      dst[dstIdx++] = dst[ref++];
	      matchLen--;
	   }

	   return dstIdx;
   }


   inline void ROLZEncoder::encodeBit(int bit)
   {
       // Calculate interval split
       const uint64 split = (((_high - _low) >> 4) * uint64(_predictor->get())) >> 8;

       // Update fields with new interval bounds
       _high -= (-bit & (_high - _low - split));
       _low += (~ - bit & (split + 1));

       // Update predictor
       _predictor->update(bit);

       // Emit unchanged first 32 bits
       while (((_low ^ _high) >> 24) == 0) {
           BigEndian::writeInt32(&_buf[_idx], int32(_high >> 32));
           _idx += 4;
           _low <<= 32;
           _high = (_high << 32) | MASK_0_32;
       }
   }


   inline int ROLZDecoder::decodeBit()
   {
       // Calculate interval split
       const uint64 mid = _low + ((((_high - _low) >> 4) * uint64(_predictor->get())) >> 8);
       int bit;

       // Update predictor
       if (mid >= _current) {
           bit = 1;
           _high = mid;
           _predictor->update(1);
       }
       else {
           bit = 0;
           _low = mid + 1;
           _predictor->update(0);
       }

       // Read 32 bits
       while (((_low ^ _high) >> 24) == 0) {
           _low = (_low << 32) & MASK_0_56;
           _high = ((_high << 32) | MASK_0_32) & MASK_0_56;
           const uint64 val = uint64(BigEndian::readInt32(&_buf[_idx])) & MASK_0_32;
           _current = ((_current << 32) | val) & MASK_0_56;
           _idx += 4;
       }

       return bit;
   }
}
#endif
