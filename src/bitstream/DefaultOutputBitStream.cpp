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

#include "DefaultOutputBitStream.hpp"
#include "../io/IOException.hpp"
#include <fstream>

using namespace kanzi;


DefaultOutputBitStream::DefaultOutputBitStream(OutputStream& os, uint bufferSize) THROW : _os(os)
{
    if (bufferSize < 1024)
        throw invalid_argument("Invalid buffer size (must be at least 1024)");

    if (bufferSize > 1 << 29)
        throw invalid_argument("Invalid buffer size (must be at most 536870912)");

    if ((bufferSize & 7) != 0)
        throw invalid_argument("Invalid buffer size (must be a multiple of 8)");

    _availBits = 64;
    _bufferSize = bufferSize;
    _buffer = new byte[_bufferSize];
    _position = 0;
    _current = 0;
    _written = 0;
    _closed = false;
}

uint DefaultOutputBitStream::writeBits(byte bits[], uint count) THROW
{
    if (isClosed() == true)
        throw BitStreamException("Stream closed", BitStreamException::STREAM_CLOSED);

    int remaining = count;
    int start = 0;

    // Byte aligned cursor ?
    if ((_availBits & 7) == 0) {
        // Fill up _current
        while ((_availBits != 64) && (remaining >= 8)) {
            writeBits(uint64(bits[start]), 8);
            start++;
            remaining -= 8;
        }

        // Copy bits array to internal buffer
        while (uint(remaining >> 3) >= _bufferSize - _position) {
            memcpy(&_buffer[_position], &bits[start], _bufferSize - _position);
            start += (_bufferSize - _position);
            remaining -= ((_bufferSize - _position) << 3);
            _position = _bufferSize;
            flush();
        }

        const int r = (remaining >> 6) << 3;

        if (r > 0) {
            memcpy(&_buffer[_position], &bits[start], r);
            start += r;
            _position += r;
            remaining -= (r << 3);
        }
    }
    else {
        // Not byte aligned
        const int r = 64 - _availBits;

        while (remaining >= 64) {
            const uint64 value = uint64(BigEndian::readLong64(&bits[start]));
            _current |= (value >> r);
            pushCurrent();
            _current = (value << (64-r));
            _availBits -= r;
            start += 8;
            remaining -= 64;
        }
    }

    // Last bytes
    while (remaining >= 8) {
        writeBits(uint64(bits[start]), 8);
        start++;
        remaining -= 8;
    }

    if (remaining > 0)
        writeBits(uint64(bits[start]) >> (8 - remaining), remaining);

    return count;
}

void DefaultOutputBitStream::close() THROW
{
    if (isClosed() == true)
        return;

    int savedBitIndex = _availBits;
    uint savedPosition = _position;
    uint64 savedCurrent = _current;

    try {
        // Push last bytes (the very last byte may be incomplete)
        int size = ((64 - _availBits) + 7) >> 3;
        pushCurrent();
        _position -= (8 - size);
        flush();
    }
    catch (BitStreamException& e) {
        // Revert fields to allow subsequent attempts in case of transient failure
        _position = savedPosition;
        _availBits = savedBitIndex;
        _current = savedCurrent;
        throw e;
    }

    try {
        _os.flush();

        if (!_os.good())
            throw BitStreamException("Write to bitstream failed.", BitStreamException::INPUT_OUTPUT);
    }
    catch (ios_base::failure& e) {
        throw BitStreamException(e.what(), BitStreamException::INPUT_OUTPUT);
    }

    _closed = true;
    _position = 0;

    // Reset fields to force a flush() and trigger an exception
    // on writeBit() or writeBits()
    _availBits = 0;
    delete[] _buffer;
    _bufferSize = 8;
    _buffer = new byte[_bufferSize];
    _written -= 64; // adjust for method written()
}


// Write buffer to underlying stream
void DefaultOutputBitStream::flush() THROW
{
    if (isClosed() == true)
        throw BitStreamException("Stream closed", BitStreamException::STREAM_CLOSED);

    try {
        if (_position > 0) {
            _os.write(reinterpret_cast<char*>(_buffer), _position);

            if (!_os.good())
                throw BitStreamException("Write to bitstream failed", BitStreamException::INPUT_OUTPUT);

            _written += (uint64(_position) << 3);
            _position = 0;
        }
    }
    catch (ios_base::failure& e) {
        throw BitStreamException(e.what(), BitStreamException::INPUT_OUTPUT);
    }
}

DefaultOutputBitStream::~DefaultOutputBitStream()
{
    close();
    delete[] _buffer;
}
