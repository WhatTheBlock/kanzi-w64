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

#include "DefaultInputBitStream.hpp"
#include "../io/IOException.hpp"

using namespace kanzi;

DefaultInputBitStream::DefaultInputBitStream(InputStream& is, uint bufferSize) THROW : _is(is)
{
    if (bufferSize < 1024)
        throw invalid_argument("Invalid buffer size (must be at least 1024)");

    if (bufferSize > 1 << 29)
        throw invalid_argument("Invalid buffer size (must be at most 536870912)");

    if ((bufferSize & 7) != 0)
        throw invalid_argument("Invalid buffer size (must be a multiple of 8)");

    _bufferSize = bufferSize;
    _buffer = new byte[_bufferSize];
    _availBits = 0;
    _maxPosition = -1;
    _position = 0;
    _current = 0;
    _read = 0;
    _closed = false;
}

DefaultInputBitStream::~DefaultInputBitStream()
{
    close();
    delete[] _buffer;
}

uint DefaultInputBitStream::readBits(byte bits[], uint count) THROW
{
    if (isClosed() == true)
        throw BitStreamException("Stream closed", BitStreamException::STREAM_CLOSED);

    if (count == 0)
       return 0;
    
    int remaining = count;
    int start = 0;

    // Byte aligned cursor ?
    if ((_availBits & 7) == 0) {
        if (_availBits == 0)
            pullCurrent();

        // Empty _current
        while ((_availBits > 0) && (remaining >= 8)) {
            bits[start] = byte(readBits(8));
            start++;
            remaining -= 8;
        }

        prefetchRead(&_buffer[_position]);

        // Copy internal buffer to bits array
        while ((remaining >> 3) > _maxPosition + 1 - _position) {
            memcpy(&bits[start], &_buffer[_position], _maxPosition + 1 - _position);
            start += (_maxPosition + 1 - _position);
            remaining -= ((_maxPosition + 1 - _position) << 3);
            readFromInputStream(_bufferSize);
        }

        const int r = (remaining >> 6) << 3;

        if (r > 0) {
            memcpy(&bits[start], &_buffer[_position], r);
            _position += r;
            start += r;
            remaining -= (r << 3);
        }
    }
    else {
        // Not byte aligned
        const int r = 64 - _availBits;

        while (remaining >= 64) {
            const uint64 v = _current & (uint64(-1) >> (64 - _availBits));
            pullCurrent();
            _availBits -= r;
            BigEndian::writeLong64(&bits[start], (v << r) | (_current >> _availBits));
            start += 8;
            remaining -= 64;
        }
    }

    // Last bytes
    while (remaining >= 8) {
        bits[start] = byte(readBits(8));
        start++;
        remaining -= 8;
    }

    if (remaining > 0)
        bits[start] = byte(readBits(remaining) << (8 - remaining));

    return count;
}

void DefaultInputBitStream::close() THROW
{
    if (isClosed() == true)
        return;

    _closed = true;

    // Reset fields to force a readFromInputStream() and trigger an exception
    // on readBit() or readBits()
    _availBits = 0;
    _maxPosition = -1;
}

int DefaultInputBitStream::readFromInputStream(uint count) THROW
{
    if (isClosed() == true)
        throw BitStreamException("Stream closed", BitStreamException::STREAM_CLOSED);

    int size = -1;

    try {
        _read += (uint64(_maxPosition + 1) << 3);
        _is.read(reinterpret_cast<char*>(_buffer), count);

        if (_is.good()) {
            size = count;
        }
        else {
            size = int(_is.gcount());

            if (!_is.eof()) {
                _position = 0;
                _maxPosition = (size <= 0) ? -1 : size - 1;
                throw BitStreamException("No more data to read in the bitstream",
                    BitStreamException::END_OF_STREAM);
            }
        }
    }
    catch (IOException& e) {
        _position = 0;
        _maxPosition = (size <= 0) ? -1 : size - 1;
        throw BitStreamException(e.what(), BitStreamException::INPUT_OUTPUT);
    }

    _position = 0;
    _maxPosition = (size <= 0) ? -1 : size - 1;
    return size;
}

// Return false when the bitstream is closed or the End-Of-Stream has been reached
bool DefaultInputBitStream::hasMoreToRead()
{
    if (isClosed() == true)
        return false;

    if ((_position < _maxPosition) || (_availBits > 0))
        return true;

    try {
        readFromInputStream(_bufferSize);
    }
    catch (BitStreamException&) {
        return false;
    }

    return true;
}

