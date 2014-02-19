/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include "channeled-json-decompressor.h"

#include <algorithm>

#include "folly/io/Cursor.h"
#include "hphp/util/gen-cpp/channeled_json_compressor_types.h"
#include "hphp/util/logger.h"


using folly::dynamic;
using folly::fbstring;
using namespace apache::thrift;


namespace HPHP {

// utility function
bool inRange(uint8_t number, uint8_t base, uint8_t length) {
  return ((number >= base) && (number < base + length));
}


ChanneledJsonDecompressor::ChanneledJsonDecompressor():
  memoStringsNext_(0), memoKeysNext_(0) {
}

ChanneledJsonDecompressor::~ChanneledJsonDecompressor() {
}

void ChanneledJsonDecompressor::readChannels(folly::io::Cursor& cursor) {
  uint32_t length;
  try {
    for (int i = 0; i < NUM_OF_CHANNELS; ++i) {
      length = cursor.readBE<uint32_t>();
      channels_.push_back(ChannelState(cursor.readFixedString(length)));
    }
  } catch (std::logic_error& ex) {
    // something wrong has happened - probably malformed file
    Logger::Warning("Malformed input file");
    throw;
  }
}

void ChanneledJsonDecompressor::read(dynamic* outObj) {
  uint8_t curType = readType();

  // Map
  if (inRange(curType, TYPE_SHORT_MAP_BASE, SHORT_TYPE_LENGTH)) {
    readMap(curType - TYPE_SHORT_MAP_BASE, outObj);
    return;
  }
  if (curType == TYPE_LONG_MAP) {
    int64_t mapLength = readVarInt(CHANNEL_MAPLEN);
    readMap(mapLength, outObj);
    return;
  }

  // Array
  if (inRange(curType, TYPE_SHORT_ARRAY_BASE, SHORT_TYPE_LENGTH)) {
    readArray(curType - TYPE_SHORT_ARRAY_BASE, outObj);
    return;
  }
  if (curType == TYPE_LONG_ARRAY) {
    int64_t arrayLength = readVarInt(CHANNEL_ARRAYLEN);
    readArray(arrayLength, outObj);
    return;
  }

  // String - different types
  if (curType == TYPE_MEMOISED_STRING) {
    int memoisedIdx = readVarInt(CHANNEL_MEMOSTRS);
    *outObj = memoStrings_[memoisedIdx];
    return;
  }

  if (inRange(curType, TYPE_SHORT_STRING_BASE, SHORT_TYPE_LENGTH)) {
    uint32_t stringLength = curType - TYPE_SHORT_STRING_BASE;
    readString(stringLength, CHANNEL_STRS, outObj);
    memoStrings_[memoStringsNext_] = outObj->asString();
    ++memoStringsNext_;
    return;
  }

  if (curType == TYPE_STRING_SIZE) {
    uint32_t stringLength = readVarInt(CHANNEL_STRLEN);
    readString(stringLength, CHANNEL_STRS, outObj);
    memoStrings_[memoStringsNext_] = outObj->asString();
    ++memoStringsNext_;
    return;
  }

  // Boolean
  if (curType == TYPE_BOOL_TRUE) {
    *outObj = true;
    return;
  }

  if (curType == TYPE_BOOL_FALSE) {
    *outObj = false;
    return;
  }

  // Integer
  if (inRange(curType, TYPE_SMALL_INT_BASE, SHORT_TYPE_LENGTH)) {
    *outObj = curType - TYPE_SMALL_INT_BASE;
    return;
  }

  if (curType == TYPE_LONG_INT) {
    *outObj = readVarInt(CHANNEL_INTS);
    return;
  }

  // None
  if (curType == TYPE_NONE) {
    *outObj = nullptr;
    return;
  }

  // Double
  if (curType == TYPE_DOUBLE) {
    readDouble(outObj);
    return;
  }

  //TODO(noamler) - unknown type error handling. Log?
  *outObj = nullptr;

}

int64_t ChanneledJsonDecompressor::readVarInt(Channel channel) {
  const char *bits = channels_[channel].read(1);
  int64_t value = 0;
  uint32_t shift = 0;

  while ((*bits & 0x80) != 0) {
    value |= (*bits & 0x7f) << shift;
    bits = channels_[channel].read(1);
    shift += 7;
  }

  value += (*bits << shift);

  //checking for an originally negative value
  if ((value & (0x8000000000000000ull)) != 0) {
    value -= UINT64_MAX + 1;
  }

  return value;
}

uint8_t ChanneledJsonDecompressor::readType() {
  const char *byteBuf = channels_[CHANNEL_TYPES].read(sizeof(uint8_t));
  return *bitwise_cast<uint8_t*>(byteBuf);
}

void ChanneledJsonDecompressor::readMap(int64_t length, dynamic* outMapObj) {
  *outMapObj = dynamic::object();
  int64_t keyType;

  for (int64_t i = 0; i < length; ++i) {
    dynamic curKey(nullptr);
    keyType = readVarInt(CHANNEL_KEYS);

    if (keyType == KEY_TYPE_STRING) {
      uint64_t keyLength = readVarInt(CHANNEL_KEYS);
      readString(keyLength, CHANNEL_KEYS, &curKey);

      // saving for memoised keys
      memoKeys_[memoKeysNext_] = curKey.asString();
      ++memoKeysNext_;
    } else { // key type memoised (KEY_TYPE_STATIC is not used yet)
      curKey = memoKeys_[keyType - KEY_TYPE_MEMOISED_BASE];
    }

    // reading value
    dynamic value(nullptr); // dummy initialization
    read(&value);

    //appending result
    outMapObj->operator[](curKey) = value;
  }
}

void ChanneledJsonDecompressor::readArray(int64_t length,
                                          dynamic* outArrayObj) {
  *outArrayObj = {};
  for (int64_t i = 0; i < length; ++i) {
    dynamic curObj(nullptr);
    read(&curObj);
    outArrayObj->push_back(curObj);
  }

}

void ChanneledJsonDecompressor::readString(int64_t length, Channel channel,
                                           dynamic* outStringObj) {
  const char *pos = channels_[channel].read(length);
  *outStringObj = fbstring(pos, length);
  //TODO(noamler) How to decode utf8? (https://fburl.com/16145985)
}

void ChanneledJsonDecompressor::readDouble(dynamic* outDoubleObj) {
  BOOST_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t));
  BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);

  const char* dubBuf = channels_[CHANNEL_DOUBLES].read(sizeof(double));
  uint64_t bits = *bitwise_cast<uint64_t*>(dubBuf);
  bits = ntohll(bits);
  *outDoubleObj = bitwise_cast<double>(bits);
}


ChanneledJsonDecompressor::ChannelState::ChannelState(const fbstring& str):
  channel_(str), pos_(0) {
}

const char* ChanneledJsonDecompressor::ChannelState::read(uint32_t length) {
  const char* retVal = channel_.data() + pos_;
  pos_ += length;
  return retVal;
}

} // namespace HPHP
