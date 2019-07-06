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

#ifndef _TextCodec_
#define _TextCodec_

#include <map>
#include "../Function.hpp"

using namespace std;

namespace kanzi {
   class DictEntry {
   public:
       int32 _hash; // full word hash
       int32 _data; // packed word length (8 MSB) + index in dictionary (24 LSB)
       const byte* _ptr; // text data

       DictEntry();

       DictEntry(const byte* ptr, int32 hash, int idx, int length);

       DictEntry(const DictEntry& de);

       DictEntry& operator=(const DictEntry& de);

       ~DictEntry() {}
   };

   // Encode word indexes using a token
   class TextCodec1 : public Function<byte> {
   public:
       TextCodec1();

       TextCodec1(map<string, string>& ctx);

       virtual ~TextCodec1()
       {
           delete[] _dictList;
           delete[] _dictMap;
       }

       bool forward(SliceArray<byte>& src, SliceArray<byte>& dst, int length);

       bool inverse(SliceArray<byte>& src, SliceArray<byte>& dst, int length);

       // Limit to 1 x srcLength and let the caller deal with
       // a failure when the output is too small
       int getMaxEncodedLength(int srcLen) const { return srcLen; }

   private:
       DictEntry** _dictMap;
       DictEntry* _dictList;
       byte _escapes[2];
       int _staticDictSize;
       int _dictSize;
       int _logHashSize;
       int32 _hashMask;
       bool _isCRLF; // EOL = CR + LF

       bool expandDictionary();
       inline void reset();
       inline int emitWordIndex(byte dst[], int val);
       inline int emitSymbols(byte src[], byte dst[], const int srcEnd, const int dstEnd);
   };

   // Encode word indexes using a mask (0x80)
   class TextCodec2 : public Function<byte> {
   public:
       TextCodec2();

       TextCodec2(map<string, string>& ctx);

       virtual ~TextCodec2()
       {
           delete[] _dictList;
           delete[] _dictMap;
       }

       bool forward(SliceArray<byte>& src, SliceArray<byte>& dst, int length);

       bool inverse(SliceArray<byte>& src, SliceArray<byte>& dst, int length);

       // Limit to 1 x srcLength and let the caller deal with
       // a failure when the output is too small
       inline int getMaxEncodedLength(int srcLen) const { return srcLen; }

   private:
       DictEntry** _dictMap;
       DictEntry* _dictList;
       int _staticDictSize;
       int _dictSize;
       int _logHashSize;
       int32 _hashMask;
       bool _isCRLF; // EOL = CR + LF

       bool expandDictionary();
       inline void reset();
       inline int emitWordIndex(byte dst[], int val, int mask);
       inline int emitSymbols(byte src[], byte dst[], const int srcEnd, const int dstEnd);
   };

   // Simple one-pass text codec. Uses a default (small) static dictionary
   // or potentially larger custom one. Generates a dynamic dictionary.
   class TextCodec : public Function<byte> {
       friend class TextCodec1;
       friend class TextCodec2;

   public:
       static const int MAX_DICT_SIZE = 1 << 19; // must be less than 1<<24
       static const int MAX_WORD_LENGTH = 32; // must be less than 128
       static const int MAX_BLOCK_SIZE = 1 << 30; // 1 GB       
       static const byte ESCAPE_TOKEN1 = byte(0x0F); // dictionary word preceded by space symbol
       static const byte ESCAPE_TOKEN2 = byte(0x0E); // toggle upper/lower case of first word char
       static const byte MASK_1F = byte(0x1F);
       static const byte MASK_20 = byte(0x20);
       static const byte MASK_40 = byte(0x40);
       static const byte MASK_80 = byte(0x80);

       TextCodec();

       TextCodec(map<string, string>& ctx);

       virtual ~TextCodec()
       {
           delete _delegate;
       }

       bool forward(SliceArray<byte>& src, SliceArray<byte>& dst, int length) THROW;
       
       bool inverse(SliceArray<byte>& src, SliceArray<byte>& dst, int length) THROW;

       int getMaxEncodedLength(int srcLen) const
       {
           return _delegate->getMaxEncodedLength(srcLen);
       }

       static bool isText(byte val) { return TEXT_CHARS[uint8(val)]; }

       static bool isLowerCase(byte val) { return (val >= byte('a')) && (val <= byte('z')); }

       static bool isUpperCase(byte val) { return (val >= byte('A')) && (val <= byte('Z')); }

       static bool isDelimiter(byte val) { return DELIMITER_CHARS[uint8(val)]; }

   private:
       static const int32 HASH1 = 0x7FEB352D;
       static const int32 HASH2 = 0x846CA68B;
       static const byte CR = byte(0x0D);
       static const byte LF = byte(0x0A);
       static const byte SP = byte(0x20);
       static const int THRESHOLD1 = 128;
       static const int THRESHOLD2 = THRESHOLD1 * THRESHOLD1;
       static const int THRESHOLD3 = 32;
       static const int THRESHOLD4 = THRESHOLD3 * 128;
       static const int LOG_HASHES_SIZE = 24; // 16 MB
       static const byte MASK_NOT_TEXT = byte(0x80);
       static const byte MASK_ALMOST_FULL_ASCII = byte(0x08);
       static const byte MASK_FULL_ASCII = byte(0x04);
       static const byte MASK_XML_HTML = byte(0x02);
       static const byte MASK_CRLF = byte(0x01);

       static bool* initDelimiterChars();
       static const bool* DELIMITER_CHARS;
       static bool* initTextChars();
       static const bool* TEXT_CHARS;

       static SliceArray<byte> unpackDictionary32(const byte dict[], int dictSize);

       static bool sameWords(const byte src[], byte dst[], const int length);

       static byte computeStats(byte block[], int count, int32 freqs[]);

       // Default dictionary
       static const byte DICT_EN_1024[];

       // Static dictionary of 1024 entries.
       static DictEntry STATIC_DICTIONARY[1024];
       static int createDictionary(SliceArray<byte> words, DictEntry dict[], int maxWords, int startWord);
       static const int STATIC_DICT_WORDS;

       Function<byte>* _delegate;
   };

   inline DictEntry::DictEntry()
   {
       _ptr = nullptr;
       _hash = 0;
       _data = 0;
   }

   inline DictEntry::DictEntry(const byte* ptr, int32 hash, int idx, int length)
   {
       _ptr = ptr;
       _hash = hash;
       _data = ((length & 0xFF) << 24) | (idx & 0x00FFFFFF);
   }

   inline DictEntry::DictEntry(const DictEntry& de)
   {
       _ptr = de._ptr;
       _hash = de._hash;
       _data = de._data;
   }

   inline DictEntry& DictEntry::operator=(const DictEntry& de)
   {
       _ptr = de._ptr;
       _hash = de._hash;
       _data = de._data;
       return *this;
   }


   inline bool TextCodec::sameWords(const byte src[], byte dst[], const int length)
   {
	   if (length >= 4) {
		   int32* p1 = (int32*)&dst[0];
		   int32* p2 = (int32*)&src[0];
		   int n = length;

		   while (n >= 4) {
			   if (*p1++ != *p2++)
				   return false;

			   n -= 4;
		   }

		   for (int i = length - n; i < length; i++) {
			   if (dst[i] != src[i])
				   return false;
		   }

		   return true;
	   }

	   for (int i = 0; i < length; i++) {
		   if (dst[i] != src[i])
			   return false;
	   }

	   return true;
   }
}
#endif
