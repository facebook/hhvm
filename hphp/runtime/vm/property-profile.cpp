/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/property-profile.h"

#include <atomic>
#include <cinttypes>
#include <tbb/concurrent_hash_map.h>

#include "hphp/runtime/base/string-functors.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"

namespace HPHP { namespace PropertyProfile {

//////////////////////////////////////////////////////////////////////

namespace {

TRACE_SET_MOD(prof_prop);

using ClassMethodPair = std::pair<const StringData*, const StringData*>;

using PropertyCounts = tbb::concurrent_hash_map<
  ClassMethodPair,
  uint32_t,
  StringDataPairHashICompare
>;

PropertyCounts s_counts;

}

//////////////////////////////////////////////////////////////////////

void incCount(const StringData* cls, const StringData* prop) {
  auto fullName = ClassMethodPair(cls, prop);

  PropertyCounts::accessor acc;
  if (!s_counts.insert(acc, PropertyCounts::value_type(fullName, 1))) {
    acc->second++;
  }
}

uint32_t getCount(const StringData* cls, const StringData* prop) {
  auto fullName = ClassMethodPair(cls, prop);

  PropertyCounts::accessor acc;
  if (s_counts.find(acc, fullName)) return acc->second;
  return 0;
}

void serialize(jit::ProfDataSerializer& ser) {
  write_raw(ser, s_counts.size());
  FTRACE(1, "PropertyProfile::serialize ({} entries):\n", s_counts.size());
  for (auto const& elm : s_counts) {
    FTRACE(1, "  {}::{} = {}\n",
           elm.first.first->data(), elm.first.second->data(), elm.second);
    write_string(ser, elm.first.first);
    write_string(ser, elm.first.second);
    write_raw(ser, elm.second);
  }
}

void deserialize(jit::ProfDataDeserializer& ser) {
  size_t elems;
  read_raw(ser, elems);
  FTRACE(1, "PropertyProfile::deserialize ({} entries):\n", elems);
  while (elems--) {
    auto const clsName = read_string(ser);
    auto const propName = read_string(ser);
    auto const fullName = ClassMethodPair(clsName, propName);
    uint32_t count;
    read_raw(ser, count);
    FTRACE(1, "  {}::{} = {}\n", clsName->data(), propName->data(), count);
    PropertyCounts::accessor acc;
    if (!s_counts.insert(acc, PropertyCounts::value_type(fullName, count))) {
      always_assert_flog(
        0, "found duplicate entry while deserializing PropertyProfile"
      );
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}
