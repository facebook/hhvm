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

#include "hphp/runtime/vm/jit/llvm-locrecs.h"
#include "hphp/util/assertions.h"

#include <folly/Format.h>

namespace HPHP { namespace jit {

namespace {
/*
 * Read a T from ptr into out, advancing ptr past the read item.
 */
template<typename T>
void readValue(const uint8_t*& ptr, T& out) {
  memcpy(&out, ptr, sizeof(T));
  ptr += sizeof(T);
}

/*
 * Advance ptr past T making sure the stored value was 0.
 */
template<typename T>
void skipValue(const uint8_t*& ptr, T& out) {
  memcpy(&out, ptr, sizeof(T));
  always_assert(out == 0 && "non-zero reserved field");
  ptr += sizeof(T);
}
}

LocRecs parseLocRecs(const uint8_t* ptr, size_t size) {
  LocRecs recs;

  uint8_t  reserved8;
  uint16_t reserved16;
  uint32_t reserved32;

  always_assert(size >= 8);
  auto const end = ptr + size;

  readValue(ptr, recs.versionMajor);
  readValue(ptr, recs.versionMinor);
  always_assert(recs.versionMajor == 1 && "invalid version of locrecs");

  skipValue(ptr, reserved16);

  uint32_t numFunctions;
  readValue(ptr, numFunctions);

  for (uint32_t i = 0; i < numFunctions; ++i) {
    always_assert(ptr + 16 <= end);
    LocRecs::FunctionRecord funcRec;
    readValue(ptr, funcRec.address);
    uint32_t numRecords;
    readValue(ptr, numRecords);
    skipValue(ptr, reserved32);

    always_assert(ptr + numRecords * 8 <= end);
    for (uint32_t j = 0; j < numRecords; ++j) {
      LocRecs::LocationRecord record;
      readValue(ptr, record.offset);
      readValue(ptr, record.id);
      readValue(ptr, record.size);
      skipValue(ptr, reserved8);
      funcRec.records[record.id].emplace_back(std::move(record));
    }
    recs.functionRecords.emplace(funcRec.address, std::move(funcRec));
  }

  return recs;
}

std::string show(const LocRecs& recs) {
  std::string ret;
  folly::format(&ret, "Major version: {}\n", recs.versionMajor);
  folly::format(&ret, "Minor version: {}\n", recs.versionMinor);

  folly::format(&ret, "FunctionRecord[{}] = {{\n",
                recs.functionRecords.size());
  for (auto const& funcRec : recs.functionRecords) {
    folly::format(&ret, "  FunctionAddress = {}\n", funcRec.second.address);
    folly::format(&ret, "  LocationRecord[{}] = {{\n",
                  funcRec.second.records.size());
    for (auto const& pair : funcRec.second.records) {
      folly::format(&ret, "  id {} = {{\n", pair.first);
      for (auto const& locrec : pair.second) {
        ret += "    {\n";
        folly::format(&ret, "      offset = {}\n", locrec.offset);
        folly::format(&ret, "      size = {}\n", locrec.size);
        ret += "    }\n";
      }
      ret += "  }\n";
    }
  }
  ret += "}\n";

  return ret;
}

} }
