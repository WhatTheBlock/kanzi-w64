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

#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "../types.hpp"
#include "../entropy/HuffmanEncoder.hpp"
#include "../entropy/RangeEncoder.hpp"
#include "../entropy/ANSRangeEncoder.hpp"
#include "../entropy/BinaryEntropyEncoder.hpp"
#include "../entropy/ExpGolombEncoder.hpp"
#include "../entropy/RiceGolombEncoder.hpp"
#include "../bitstream/DefaultOutputBitStream.hpp"
#include "../bitstream/DefaultInputBitStream.hpp"
#include "../bitstream/DebugOutputBitStream.hpp"
#include "../bitstream/DebugInputBitStream.hpp"
#include "../entropy/HuffmanDecoder.hpp"
#include "../entropy/RangeDecoder.hpp"
#include "../entropy/ANSRangeDecoder.hpp"
#include "../entropy/BinaryEntropyDecoder.hpp"
#include "../entropy/ExpGolombDecoder.hpp"
#include "../entropy/RiceGolombDecoder.hpp"
#include "../entropy/FPAQPredictor.hpp"
#include "../entropy/CMPredictor.hpp"
#include "../entropy/TPAQPredictor.hpp"

using namespace kanzi;

static Predictor* getPredictor(string type)
{
    if (type.compare("TPAQ") == 0)
        return new TPAQPredictor<false>();

    if (type.compare("TPAQX") == 0)
        return new TPAQPredictor<true>();

    if (type.compare("FPAQ") == 0)
        return new FPAQPredictor();

    if (type.compare("CM") == 0)
        return new CMPredictor();

    return nullptr;
}

static EntropyEncoder* getEncoder(string name, OutputBitStream& obs, Predictor* predictor)
{
    if (name.compare("HUFFMAN") == 0)
        return new HuffmanEncoder(obs);

    if (name.compare("ANS0") == 0)
        return new ANSRangeEncoder(obs, 0);

    if (name.compare("ANS1") == 0)
        return new ANSRangeEncoder(obs, 1);

    if (name.compare("RANGE") == 0)
        return new RangeEncoder(obs);

    if (name.compare("EXPGOLOMB") == 0)
        return new ExpGolombEncoder(obs);

    if (name.compare("RICEGOLOMB") == 0)
        return new RiceGolombEncoder(obs, 4);

    if (predictor != nullptr) {
       if (name.compare("TPAQ") == 0)
           return new BinaryEntropyEncoder(obs, predictor, false);

       if (name.compare("FPAQ") == 0)
           return new BinaryEntropyEncoder(obs, predictor, false);

       if (name.compare("CM") == 0)
           return new BinaryEntropyEncoder(obs, predictor, false);
    }

    cout << "No such entropy encoder: " << name << endl;
    return nullptr;
}

static EntropyDecoder* getDecoder(string name, InputBitStream& ibs, Predictor* predictor)
{
    if (name.compare("HUFFMAN") == 0)
        return new HuffmanDecoder(ibs);

    if (name.compare("ANS0") == 0)
        return new ANSRangeDecoder(ibs, 0);

    if (name.compare("ANS1") == 0)
        return new ANSRangeDecoder(ibs, 1);

    if (name.compare("RANGE") == 0)
        return new RangeDecoder(ibs);

    if (name.compare("TPAQ") == 0)
        return new BinaryEntropyDecoder(ibs, predictor, false);

    if (name.compare("TPAQX") == 0)
        return new BinaryEntropyDecoder(ibs, predictor, false);

    if (name.compare("FPAQ") == 0)
        return new BinaryEntropyDecoder(ibs, predictor, false);

    if (name.compare("CM") == 0)
        return new BinaryEntropyDecoder(ibs, predictor, false);

    if (name.compare("EXPGOLOMB") == 0)
        return new ExpGolombDecoder(ibs);

    if (name.compare("RICEGOLOMB") == 0)
        return new RiceGolombDecoder(ibs, 4);

    cout << "No such entropy decoder: " << name << endl;
    return nullptr;
}

int testEntropyCodecCorrectness(const string& name)
{
    // Test behavior
    cout << "Correctness test for " << name << endl;
    srand((uint)time(nullptr));
    int res = 0;

    for (int ii = 1; ii < 20; ii++) {
        cout << endl
             << endl
             << "Test " << ii << endl;
        byte val[256];
        int size = 32;

        if (ii == 3) {
            byte val2[] = { (byte)0, (byte)0, (byte)32, (byte)15, (byte)-4, (byte)16, (byte)0, (byte)16, (byte)0, (byte)7, (byte)-1, (byte)-4, (byte)-32, (byte)0, (byte)31, (byte)-1 };
            size = 16;
            memcpy(val, &val2[0], size);
        }
        else if (ii == 2) {
            byte val2[] = { (byte)0x3d, (byte)0x4d, (byte)0x54, (byte)0x47, (byte)0x5a, (byte)0x36, (byte)0x39, (byte)0x26, (byte)0x72, (byte)0x6f, (byte)0x6c, (byte)0x65, (byte)0x3d, (byte)0x70, (byte)0x72, (byte)0x65 };
            size = 16;
            memcpy(val, &val2[0], size);
        }
        else if (ii == 1) {
            for (int i = 0; i < 32; i++)
                val[i] = byte(2); // all identical
        }
        else if (ii == 5) {
            for (int i = 0; i < 32; i++)
                val[i] = byte(2 + (i & 1)); // 2 symbols
        }
        else {
            size = 256;
            
            for (int i = 0; i < 256; i++)
                val[i] = byte(64 + 4 * ii + (rand() % (8*ii + 1)));
        }

        byte* values = &val[0];
        cout << "Original:" << endl;

        for (int i = 0; i < size; i++)
            cout << int(values[i]) << " ";

        cout << endl
             << endl
             << "Encoded:" << endl;
        stringbuf buffer;
        iostream ios(&buffer);
        DefaultOutputBitStream obs(ios);
        DebugOutputBitStream dbgobs(obs);
        dbgobs.showByte(true);

        EntropyEncoder* ec = getEncoder(name, dbgobs, getPredictor(name));

        if (ec == nullptr)
           return 1;

        ec->encode(values, 0, size);
        ec->dispose();
        delete ec;
        dbgobs.close();
        ios.rdbuf()->pubseekpos(0);

        DefaultInputBitStream ibs(ios);
        EntropyDecoder* ed = getDecoder(name, ibs, getPredictor(name));
        
        if (ec == nullptr)
           return 1;

        cout << endl
             << endl
             << "Decoded:" << endl;
        bool ok = true;
        byte* values2 = new byte[size];
        ed->decode(values2, 0, size);
        ed->dispose();
        delete ed;
        ibs.close();

        for (int j = 0; j < size; j++) {
            if (values[j] != values2[j])
                ok = false;

            cout << (int)values2[j] << " ";
        }

        cout << endl;
        cout << ((ok) ? "Identical" : "Different") << endl;
        delete[] values2;
    }

    return res;
}

int testEntropyCodecSpeed(const string& name)
{
    // Test speed
    cout << endl
         << endl
         << "Speed test for " << name << endl;
    int repeats[] = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3 };
    int size = 500000;
    int iter = 100;
    int res = 0;

    srand((uint)time(nullptr));
    Predictor* predictor;
    byte values1[500000];
    byte values2[500000];

    for (int jj = 0; jj < 3; jj++) {
        cout << endl
             << "Test " << (jj + 1) << endl;
        double delta1 = 0, delta2 = 0;

        for (int ii = 0; ii < iter; ii++) {
            int idx = 0;

            for (int i = 0; i < size; i++) {
                int i0 = i;
                int len = repeats[idx];
                idx = (idx + 1) & 0x0F;
                byte b = (byte)(rand() % 255);

                if (i0 + len >= size)
                    len = size - i0 - 1;

                for (int j = i0; j < i0 + len; j++) {
                    values1[j] = b;
                    i++;
                }
            }

            // Encode
            stringbuf buffer;
            iostream ios(&buffer);
            DefaultOutputBitStream obs(ios, 16384);
            predictor = getPredictor(name);
            EntropyEncoder* ec = getEncoder(name, obs, predictor);
            
            if (ec == nullptr)
                 return 1;

            clock_t before1 = clock();

            if (ec->encode(values1, 0, size) < 0) {
                cout << "Encoding error" << endl;
                delete ec;
                return 1;
            }

            ec->dispose();
            clock_t after1 = clock();
            delta1 += (after1 - before1);
            delete ec;
            obs.close();

            if (predictor)
                delete predictor;

            // Decode
            ios.rdbuf()->pubseekpos(0);
            DefaultInputBitStream ibs(ios, 16384);
            predictor = getPredictor(name);
            EntropyDecoder* ed = getDecoder(name, ibs, predictor);
            
            if (ed == nullptr)
                 return 1;

            clock_t before2 = clock();

            if (ed->decode(values2, 0, size) < 0) {
                cout << "Decoding error" << endl;
                delete ed;
                return 1;
            }

            ed->dispose();
            clock_t after2 = clock();
            delta2 += (after2 - before2);
            delete ed;
            ibs.close();

            if (predictor)
                delete predictor;

            // Sanity check
            for (int i = 0; i < size; i++) {
                if (values1[i] != values2[i]) {
                    cout << "Error at index " << i << " (" << (int)values1[i] << "<->" << (int)values2[i] << ")" << endl;
                    res = 1;
                    break;
                }
            }
        }

        double prod = (double)iter * (double)size;
        double b2KB = (double)1 / (double)1024;
        double d1_sec = (double)delta1 / CLOCKS_PER_SEC;
        double d2_sec = (double)delta2 / CLOCKS_PER_SEC;
        cout << "Encode [ms]       : " << (int)(d1_sec * 1000) << endl;
        cout << "Throughput [KB/s] : " << (int)(prod * b2KB / d1_sec) << endl;
        cout << "Decode [ms]       : " << (int)(d2_sec * 1000) << endl;
        cout << "Throughput [KB/s] : " << (int)(prod * b2KB / d2_sec) << endl;
    }

    return res;
}

#ifdef __GNUG__
int main(int argc, const char* argv[])
#else
int TestEntropyCodec_main(int argc, const char* argv[])
#endif
{
    int res = 0;

    try {
        string str;

        if (argc == 1) {
            str = "-TYPE=ALL";
        }
        else {
            str = argv[1];
        }

        transform(str.begin(), str.end(), str.begin(), ::toupper);

        if (str.compare(0, 6, "-TYPE=") == 0) {
            str = str.substr(6);

            if (str.compare("ALL") == 0) {
                cout << endl
                     << endl
                     << "TestHuffmanCodec" << endl;
                res |= testEntropyCodecCorrectness("HUFFMAN");
                res |= testEntropyCodecSpeed("HUFFMAN");
                cout << endl
                     << endl
                     << "TestANS0Codec" << endl;
                res |= testEntropyCodecCorrectness("ANS0");
                res |= testEntropyCodecSpeed("ANS0");
                cout << endl
                     << endl
                     << "TestANS1Codec" << endl;
                res |= testEntropyCodecCorrectness("ANS1");
                res |= testEntropyCodecSpeed("ANS1");
                cout << endl
                     << endl
                     << "TestRangeCodec" << endl;
                res |= testEntropyCodecCorrectness("RANGE");
                res |= testEntropyCodecSpeed("RANGE");
                cout << endl
                     << endl
                     << "TestFPAQCodec" << endl;
                res |= testEntropyCodecCorrectness("FPAQ");
                res |= testEntropyCodecSpeed("FPAQ");
                cout << endl
                     << endl
                     << "TestCMCodec" << endl;
                res |= testEntropyCodecCorrectness("CM");
                res |= testEntropyCodecSpeed("CM");
                cout << endl
                     << endl
                     << "TestTPAQCodec" << endl;
                res |= testEntropyCodecCorrectness("TPAQ");
                res |= testEntropyCodecSpeed("TPAQ");
                cout << endl
                     << endl
                     << "TestExpGolombCodec" << endl;
                res |= testEntropyCodecCorrectness("EXPGOLOMB");
                res |= testEntropyCodecSpeed("EXPGOLOMB");
                cout << endl
                     << endl
                     << "TestRiceGolombCodec" << endl;
                res |= testEntropyCodecCorrectness("RICEGOLOMB");
                res |= testEntropyCodecSpeed("RICEGOLOMB");
            }
            else {
                cout << endl
                     << endl
                     << "Test" << str << "EntropyCodec" << endl;
                res |= testEntropyCodecCorrectness(str);
                res |= testEntropyCodecSpeed(str);
            }
        }
    }
    catch (exception& e) {
        cout << e.what() << endl;
    }

    return res;
}
