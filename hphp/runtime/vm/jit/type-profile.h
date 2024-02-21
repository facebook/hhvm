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

#pragma once

#include <folly/json/dynamic.h>

#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/type-scan.h"

#include <string>

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * TypeProfile keeps the union of all the types observed during profiling.
 */
struct TypeProfile {
  std::string toString() const { return type.toString(); }

  folly::dynamic toDynamic() const {
    return folly::dynamic::object("profileType", "TypeProfile")
                                 ("typeStr", type.toString())
                                 ("count", count);
  }

  void report(TypedValue tv) {
    type |= typeFromTV(&tv, nullptr);
    count++;
  }

  static void reduce(TypeProfile& a, const TypeProfile& b) {
    a.type |= b.type;
    a.count += b.count;
  }

  void serialize(ProfDataSerializer& ser) const {
    type.serialize(ser);
    write_raw(ser, count);
  }

  void deserialize(ProfDataDeserializer& ser) {
    type = Type::deserialize(ser);
    count = read_raw<uint32_t>(ser);
  }

  Type type; // This gets initialized with 0, which is TBottom.
  uint32_t count; // zero initialized
  static_assert(Type::kBottom.empty(), "Assuming TBottom is 0");

  // In RDS, but can't contain pointers to request-allocated data.
  TYPE_SCAN_IGNORE_ALL;
};

///////////////////////////////////////////////////////////////////////////////

}
