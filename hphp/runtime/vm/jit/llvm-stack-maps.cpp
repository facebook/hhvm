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

#include "hphp/runtime/vm/jit/llvm-stack-maps.h"

#include <folly/Format.h>

#include "hphp/util/assertions.h"

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
}

StackMaps parseStackMaps(const uint8_t* ptr) {
  StackMaps maps;

  // Format defined at http://llvm.org/docs/StackMaps.html#stack-map-format
  readValue(ptr, maps.version);
  uint8_t reserved8;
  readValue(ptr, reserved8);
  assertx(reserved8 == 0);
  uint16_t reserved16;
  readValue(ptr, reserved16);
  assertx(reserved16 == 0);

  uint32_t numFunctions, numConstants, numRecords;
  readValue(ptr, numFunctions);
  readValue(ptr, numConstants);
  readValue(ptr, numRecords);
  maps.stkSizeRecords.reserve(numFunctions);
  maps.constants.reserve(numConstants);
  maps.records.reserve(numRecords);

  for (uint32_t i = 0; i < numFunctions; ++i) {
    StackMaps::StkSizeRecord record;
    readValue(ptr, record.funcAddr);
    readValue(ptr, record.stkSize);
    maps.stkSizeRecords.emplace_back(record);
  }

  for (uint32_t i = 0; i < numConstants; ++i) {
    uint64_t constant;
    readValue(ptr, constant);
    maps.constants.emplace_back(constant);
  }

  for (uint32_t i = 0; i < numRecords; ++i) {
    StackMaps::Record record;
    readValue(ptr, record.id);
    readValue(ptr, record.offset);
    ptr += sizeof(uint16_t); // Reserved (record flags)

    uint16_t numLocations;
    readValue(ptr, numLocations);
    for (uint16_t j = 0; j < numLocations; ++j) {
      using Location = StackMaps::Record::Location;
      Location loc;
      readValue(ptr, loc.kind);
      assertx(loc.kind >= Location::Kind::Register &&
             loc.kind <= Location::Kind::ConstIndex);

      ptr += sizeof(uint8_t); // Reserved (location flags)
      readValue(ptr, loc.regNum);
      readValue(ptr, loc.offsetOrConst);
      record.locations.emplace_back(loc);
    }

    ptr += sizeof(uint16_t); // Padding
    uint16_t numLiveOuts;
    readValue(ptr, numLiveOuts);
    for (uint16_t j = 0; j < numLiveOuts; ++j) {
      StackMaps::Record::LiveOut liveOut;
      readValue(ptr, liveOut.regNum);
      ptr += sizeof(uint8_t); // Reserved
      readValue(ptr, liveOut.size);
      record.liveOuts.emplace_back(liveOut);
    }

    if ((reinterpret_cast<intptr_t>(ptr) % 8) != 0) ptr += sizeof(uint32_t);
    assertx((reinterpret_cast<intptr_t>(ptr) % 8) == 0);

    maps.records.emplace(record.id, std::move(record));
  }

  return maps;
}

std::string show(const StackMaps& maps) {
  std::string ret;
  folly::format(&ret, "Version: {}\n", maps.version);

  folly::format(&ret, "StkSizeRecord[{}] = {{\n", maps.stkSizeRecords.size());
  for (auto& record : maps.stkSizeRecords) {
    folly::format(&ret, "  funcAddr = {}, stkSize = {}\n",
                  record.funcAddr, record.stkSize);
  }
  ret += "}\n";

  folly::format(&ret, "Constant[{}] = {{\n", maps.constants.size());
  for (auto& constant : maps.constants) {
    folly::format(&ret, "  {},\n", constant);
  }
  ret += "}\n";

  folly::format(&ret, "Record[{}] = {{\n", maps.records.size());
  for (auto& pair : maps.records) {
    auto& record = pair.second;

    ret += "  {\n";
    folly::format(&ret, "    id = {}\n", record.id);
    folly::format(&ret, "    offset = {}\n", record.offset);
    folly::format(&ret, "    Location[{}] = ...\n", record.locations.size());
    folly::format(&ret, "    LiveOuts[{}] = ...\n", record.liveOuts.size());
    ret += "  },\n";
  }
  ret += "}\n";

  return ret;
}

} }
