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
#include "hphp/util/channeled-json-decompressor.h"

#include "folly/json.h"
#include "folly/io/IOBufQueue.h"
#include "folly/io/Cursor.h"

#include <gtest/gtest.h>
#include <fstream>
#include <memory>


using folly::dynamic;
using folly::toJson;
using folly::fbstring;

namespace HPHP {

// Dummy test for future extensions
class TestChanneledJsonCompressor : public testing::Test {
};

void dumpToOutputFile(const std::string& outputFilename,
  folly::IOBufQueue& bufQueue) {
  std::ofstream ofs(outputFilename, std::ios::binary);
  folly::io::Cursor cursor(bufQueue.front());
  uint32_t networkLength, length;
  fbstring curChannel;

  for (int i = 0; i < NUM_OF_CHANNELS; ++i) {
    networkLength = cursor.read<uint32_t>();
    ofs.write((const char*) &networkLength, sizeof(networkLength));

    length = ntohl(networkLength);
    curChannel = cursor.readFixedString(length);
    ofs.write(curChannel.data(), curChannel.size());
  }

   ofs.close();
}

TEST(TestChanneledJsonCompressor, BaseTest) {

  // shamelessly using the same json in the jsonTest
  dynamic value = dynamic::object
    ("foo", "bar")
    ("junk", 12)
    ("another", 32.2)
    ("a",
      {
        dynamic::object("a", "b")
                       ("c", "d"),
        12.5,
        "Yo Dawg",
        { "heh" },
        nullptr
      }
    )
    ;

  fbstring jsonString = toJson(value);
  ChanneledJsonCompressor jsonCompressor;
  jsonCompressor.processJson(jsonString.c_str(), jsonString.size());

  folly::IOBufQueue bufQueue;
  jsonCompressor.finalize(bufQueue);

  folly::io::Cursor cursor(bufQueue.front());

  ChanneledJsonDecompressor jsonDecompressor;
  jsonDecompressor.readChannels(cursor);
  dynamic result(nullptr);
  jsonDecompressor.read(&result);

  ASSERT_EQ(value, result);

}

} // namespace HPHP
