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

#ifndef incl_HPHP_JIT_TYPE_SOURCE_H_
#define incl_HPHP_JIT_TYPE_SOURCE_H_

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/util/assertions.h"

#include <string>

namespace HPHP { namespace jit {

struct IRInstruction;
struct SSATmp;

/*
 * The source of a local's type. Either the current value or a guard.
 *
 * Used for guard relaxation, as we're constraining a value, local, or stack
 * slot, and we need to know what guards that type came from.
 */
struct TypeSource {
  enum class Kind : uint8_t {
    Value,
    Guard,
  };

  static TypeSource makeValue(SSATmp* value) {
    assertx(value);
    TypeSource src;
    src.value = value;
    src.kind = Kind::Value;
    return src;
  }

  static TypeSource makeGuard(const IRInstruction* guard) {
    assertx(guard);
    TypeSource src;
    src.guard = guard;
    src.kind = Kind::Guard;
    return src;
  }

  bool isGuard() const { return kind == Kind::Guard; }
  bool isValue() const { return kind == Kind::Value; }

  bool operator==(const TypeSource& rhs) const {
    return kind == rhs.kind && value == rhs.value;
  }
  bool operator!=(const TypeSource& rhs) const {
    return !operator==(rhs);
  }
  bool operator<(const TypeSource& rhs) const;

  std::string toString() const;

  // Members.
  union {
    SSATmp* value{nullptr};
    const IRInstruction* guard;
  };

  Kind kind;
};

typedef jit::flat_set<TypeSource> TypeSourceSet;

std::string show(const TypeSource&);
std::string show(const TypeSourceSet&);

}}

#endif
