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
#include "hphp/util/channeled-json-compressor.h"

#include "thrift/lib/cpp/util/BitwiseCast.h" //nolint
#include "thrift/lib/cpp/protocol/TProtocol.h" //nolint
#include "hphp/util/gen-cpp/channeled_json_compressor_types.h"
#include "folly/json.h"
#include "folly/MapUtil.h"

#define JSON_CHANNELED_CODING CODING_GZIP

using folly::dynamic;
using folly::fbstring;
using namespace apache::thrift;

namespace HPHP {

ChanneledJsonCompressor::ChanneledJsonCompressor():
  memoStringsNext_(0), memoKeysNext_(0) {
  // setting the number of channels
  channels_.resize(NUM_OF_CHANNELS);
}


ChanneledJsonCompressor::~ChanneledJsonCompressor() {
}


void ChanneledJsonCompressor::processJson(const char* data, int length) {
  folly::json::serialization_opts opts;
  opts.encode_non_ascii = true; //encodes non ascii characters
  dynamic completeJson = folly::parseJson(data, opts);
  write(completeJson);
}

void ChanneledJsonCompressor::write(const dynamic& obj) {
  switch (obj.type()) {

  case dynamic::OBJECT:
    writeMap(obj);
    break;

  case dynamic::ARRAY:
    writeArray(obj);
    break;

  case dynamic::STRING:
    writeString(obj);
    break;

  case dynamic::BOOL:
    if (obj.asBool()) {
      writeType(TYPE_BOOL_TRUE);
    } else {
      writeType(TYPE_BOOL_FALSE);
    }
    break;

  case dynamic::INT64: {
    int64_t asInt = obj.asInt();
    if ((asInt >=0) && (asInt < SHORT_TYPE_LENGTH)) {
      writeType(TYPE_SMALL_INT_BASE + asInt);
    } else {
      writeType(TYPE_LONG_INT);
      writeVarInt(CHANNEL_INTS, asInt);
    }}
    break;

  case dynamic::NULLT:
    writeType(TYPE_NONE);
    break;

  case dynamic::DOUBLE:
    writeDouble(obj);
    break;
  }
}


void ChanneledJsonCompressor::writeVarInt(Channel channel, int64_t value) {
  uint64_t bigValue = 0;
  if (value >= 0) {
    bigValue = value;
  } else {
    bigValue = UINT64_MAX + value + 1;
  }

  uint8_t bits = bigValue & 0x7f;
  uint8_t toWrite;
  bigValue >>= 7;
  while (bigValue > 0) {
    toWrite = bits | 0x80;
    channels_[channel].push_back(toWrite);
    bits = bigValue & 0x7f;
    bigValue >>= 7;
  }
  channels_[channel].push_back(bits);
}


void ChanneledJsonCompressor::writeType(uint8_t jsonType) {
  channels_[CHANNEL_TYPES].push_back(jsonType);
}


void ChanneledJsonCompressor::writeMap(const dynamic& mapObj) {
  // writing length
  if (mapObj.size() < SHORT_TYPE_LENGTH) {
    writeType(TYPE_SHORT_MAP_BASE + mapObj.size());
  } else {
    writeType(TYPE_LONG_MAP);
    writeVarInt(CHANNEL_MAPLEN, mapObj.size());
  }

  // writing (key, value) pairs
  for (auto& pair : mapObj.items()) {

    // Future reference - there might be a use of KEY_TYPE_STATIC at some point
    int *memoisedKey = get_ptr(memoKeys_, pair.first.c_str());
    if (memoisedKey != nullptr) { //if memoised
      writeVarInt(CHANNEL_KEYS, KEY_TYPE_MEMOISED_BASE + *memoisedKey);
    } else {
      writeVarInt(CHANNEL_KEYS, KEY_TYPE_STRING);
      writeVarInt(CHANNEL_KEYS, pair.first.size());
      channels_[CHANNEL_KEYS].append(pair.first.c_str());

      memoKeys_[pair.first.c_str()] = memoKeysNext_;
      ++memoKeysNext_;
    }
    //value
    write(pair.second);
  }
}


void ChanneledJsonCompressor::writeArray(const folly::dynamic& arrayObj) {
  if (arrayObj.size() < SHORT_TYPE_LENGTH) {
    writeType(TYPE_SHORT_ARRAY_BASE + arrayObj.size());
  } else {
    writeType(TYPE_LONG_ARRAY);
    writeVarInt(CHANNEL_ARRAYLEN, arrayObj.size());
  }

  for (auto& val : arrayObj) {
    write(val);
  }
}

void ChanneledJsonCompressor::writeString(const dynamic& stringObj) {
  // strings were already utf-8 encoded when used the serialization options
  // in parseJson

  //check if string has been already memoised
  int *memoisedIdx = get_ptr(memoStrings_, stringObj.c_str());
  if (memoisedIdx != nullptr) {
    writeType(TYPE_MEMOISED_STRING);
    writeVarInt(CHANNEL_MEMOSTRS, *memoisedIdx);
  } else {
    memoStrings_[stringObj.c_str()] = memoStringsNext_;
    ++memoStringsNext_;

    if (stringObj.size() < SHORT_TYPE_LENGTH) {
      writeType(TYPE_SHORT_STRING_BASE + stringObj.size());
    } else {
      writeType(TYPE_STRING_SIZE);
      writeVarInt(CHANNEL_STRLEN, stringObj.size());
    }
    channels_[CHANNEL_STRS].append(stringObj.c_str());
  }
}


void ChanneledJsonCompressor::writeDouble(const dynamic& doubleObj) {
  BOOST_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t));
  BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);

  writeType(TYPE_DOUBLE);
  double asDouble = doubleObj.asDouble();
  uint64_t bits = bitwise_cast<uint64_t>(asDouble);

  // converting to network order
  bits = htonll(bits);

  channels_[CHANNEL_DOUBLES].append((const char*)&bits, sizeof(bits));
}


void ChanneledJsonCompressor::finalize(folly::IOBufQueue& outBufQueue) {

  uint32_t networkLength;
  int length;

  for (const fbstring& channel : channels_) {

    length = channel.size();
    networkLength = htonl(length);

    // adding length
    outBufQueue.append((const void *)&networkLength, sizeof(networkLength));

    // adding channel bytes
    outBufQueue.append((const void *)channel.data(), channel.length());
  }
}


} // namesapce HPHP
