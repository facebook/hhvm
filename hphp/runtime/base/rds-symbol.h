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

#include "hphp/runtime/base/types.h"

#include "hphp/util/low-ptr.h"

#include <boost/variant.hpp>

#include <string>
#include <type_traits>

namespace HPHP {

struct Class;
struct StringData;

namespace jit {

struct ArrayAccessProfile;
struct ArrayIterProfile;
struct CallTargetProfile;
struct ClsCnsProfile;
struct DecRefProfile;
struct IncRefProfile;
struct IsTypeStructProfile;
struct MethProfile;
struct SwitchProfile;
struct TypeProfile;

}

namespace rds {

///////////////////////////////////////////////////////////////////////////////
/*
 * RDS symbols are centrally registered here.
 *
 * All StringData*'s below must be static strings.
 */

/*
 * Symbols for rds::Link's.
 */
struct LinkID { const char* type; };
struct LinkName { const char* type; LowStringPtr name; };

/*
 * Class constant values are TypedValue's stored in RDS.
 */
struct ClsConstant { LowStringPtr clsName;
                     LowStringPtr cnsName; };

/*
 * StaticMethod{F,}Cache allocations.
 *
 * These are used to cache static method dispatch targets in a given class
 * context.  The `name' field here is a string that encodes the target class,
 * property, and source context.
 */
struct StaticMethod  { LowStringPtr name; };
struct StaticMethodF { LowStringPtr name; };

/*
 * Profiling translations may store various kinds of junk under symbols that
 * are keyed on translation id.
 *
 * These generally should go in Mode::Local or Mode::Persistent, depending on
 * the use case.
 */
#define RDS_PROFILE_SYMBOLS   \
  PR(ArrayAccessProfile)  \
  PR(ArrayIterProfile)    \
  PR(CallTargetProfile)   \
  PR(ClsCnsProfile)       \
  PR(DecRefProfile)       \
  PR(IsTypeStructProfile) \
  PR(IncRefProfile)       \
  PR(MethProfile)         \
  PR(SwitchProfile)       \
  PR(TypeProfile)

enum class ProfileKind {
  None = 0,
#define PR(T) T,
  RDS_PROFILE_SYMBOLS
#undef PR
};

struct Profile {
  Profile() = default;

  Profile(TransID transId, Offset bcOff, const StringData* name)
    : kind{ProfileKind::None}
    , transId{transId}
    , bcOff{bcOff}
    , name{name}
  {}

#define PR(T) \
  Profile(const jit::T*, TransID transId,       \
          Offset bcOff, const StringData* name) \
    : kind{ProfileKind::T}                      \
    , transId{transId}                          \
    , bcOff{bcOff}                              \
    , name{name}                                \
  {}
  RDS_PROFILE_SYMBOLS
#undef PR

  ProfileKind kind;
  TransID transId;
  Offset bcOff;
  LowStringPtr name;
};

/*
 * Static class properties in Mode::Local.
 */
struct SPropCache { LowPtr<const Class> cls; Slot slot; };

struct StaticMemoValue { FuncId funcId; };
struct StaticMemoCache { FuncId funcId; };

struct LSBMemoValue {
  LowPtr<const Class> cls;
  FuncId funcId;
};

struct LSBMemoCache {
  LowPtr<const Class> cls;
  FuncId funcId;
};

struct TSCache {
  FuncId funcId;
};

using Symbol = boost::variant<
  LinkName,
  LinkID,
  ClsConstant,
  StaticMethod,
  StaticMethodF,
  Profile,
  SPropCache,
  StaticMemoValue,
  StaticMemoCache,
  LSBMemoValue,
  LSBMemoCache,
  TSCache
>;

std::string symbol_kind(const Symbol&);
std::string symbol_rep(const Symbol&);
bool symbol_eq(const Symbol&, const Symbol&);
size_t symbol_hash(const Symbol&);
size_t symbol_stable_hash(const Symbol&);

///////////////////////////////////////////////////////////////////////////////

}}
