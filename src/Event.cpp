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

#include <sstream>
#include <ctime>
#include <cstdio>
#include "Event.hpp"

using namespace kanzi;

Event::Event(Event::Type type, int id, int64 size, clock_t evtTime)
    : _type(type)
    , _time(evtTime)
    , _msg()
{
    _id = id;
    _size = size;
    _hash = 0;
    _hashing = false;
}

Event::Event(Event::Type type, int id, const string& msg, clock_t evtTime)
    : _type(type)
    , _time(evtTime)
    , _msg(msg)
{
    _id = id;
    _size = 0;
    _hash = 0;
    _hashing = false;
}

Event::Event(Event::Type type, int id, int64 size, int hash, bool hashing, clock_t evtTime)
    : _type(type)
    , _time(evtTime)
    , _msg()
{
    _id = id;
    _size = size;
    _hash = hash;
    _hashing = hashing;
}

string Event::toString() const
{
    if (_msg.size() > 0)
        return _msg;

    std::stringstream ss;
    ss << "{ \"type\":\"" << getTypeAsString() << "\"";

    if (_id >= 0)
        ss << ", \"id\":" << getId();

    ss << ", \"size\":" << getSize();
    ss << ", \"time\":" << getTime();

    if (_hashing == true) {
        char buf[32];
        sprintf(buf, "%08X", getHash());
        ss << ", \"hash\":" << buf;
    }

    ss << " }";
    return ss.str();
}

string Event::getTypeAsString() const
{
    switch (_type) {
    case COMPRESSION_START:
        return "COMPRESSION_START";

    case COMPRESSION_END:
        return "COMPRESSION_END";

    case BEFORE_TRANSFORM:
        return "BEFORE_TRANSFORM";

    case AFTER_TRANSFORM:
        return "AFTER_TRANSFORM";

    case BEFORE_ENTROPY:
        return "BEFORE_ENTROPY";

    case AFTER_ENTROPY:
        return "AFTER_ENTROPY";

    case DECOMPRESSION_START:
        return "DECOMPRESSION_START";

    case DECOMPRESSION_END:
        return "DECOMPRESSION_END";

    default:
        return "Unknown Type";
    }
}