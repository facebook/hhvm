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
#ifndef incl_HPHP_JSON_CHANNELED_DECOMPRESSOR_H_
#define incl_HPHP_JSON_CHANNELED_DECOMPRESSOR_H_

#include <vector>
#include <map>

#include "folly/dynamic.h"
#include "folly/FBString.h"
#include "folly/FBVector.h"
#include "folly/io/Cursor.h"

#include "hphp/util/gen-cpp/channeled_json_compressor_types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
A class that compresses json files according to channels
*/
class ChanneledJsonDecompressor {

public:
  ChanneledJsonDecompressor();
  ~ChanneledJsonDecompressor();

  // Reads the data into channels
  void readChannels(folly::io::Cursor& cursor);

  // Read an object from the underlying channels
  void read(folly::dynamic* outObj);

private:
  // Read a variable length integer into from the given channels
  int64_t readVarInt(Channel channel);

  // Read a type (The type is not Type because there are intermediate values)
  uint8_t readType();

  // read a map
  void readMap(int64_t length, folly::dynamic* outMapObj);

  // write an array
  void readArray(int64_t length, folly::dynamic* outArrayObj);

  // write a string
  void readString(int64_t length, Channel channel,
    folly::dynamic* outStringObj);

  // write a double
  void readDouble(folly::dynamic* outDoubleObj);

private:

  // Maintains the channel's state
  struct ChannelState {
    const folly::fbstring channel_;
    uint32_t pos_;

    // Contructor
    explicit ChannelState(const folly::fbstring& channel);

    // Get the data pointer, advance the position
    const char* read(uint32_t length);
  };

  // output channels (implemented as strings).
  // see this discussion of stringstream here: https://fburl.com/16088028
  folly::fbvector<ChannelState> channels_;

  // memoised strings
  std::map<uint32_t, folly::fbstring> memoStrings_;
  uint32_t memoStringsNext_;

  //memoised keys
  std::map<uint32_t, folly::fbstring> memoKeys_;
  uint32_t memoKeysNext_;
};

} // namespace HPHP

#endif
