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

#ifndef incl_HPHP_MEMBER_REFLECTION_H_
#define incl_HPHP_MEMBER_REFLECTION_H_

#include "hphp/runtime/base/mixed-array.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

#define HPHP_REFLECTABLES_UNQ \
  X(TypedValue)\
  /* vm metadata */\
  X(Func)\
  X(Class)\
  /* arrays */\
  X(ArrayData)\
  X(MixedArray)\
  X(EmptyArray)\
  X(APCLocalArray)\
  X(GlobalsArray)\
  X(ProxyArray)\
  /* other php types */\
  X(StringData)\
  X(ObjectData)\
  X(ResourceData)\
  X(RefData)\
  /* collections */\
  X(c_Pair)\
  X(BaseVector)\
  X(HashCollection)\
  /* */

#define HPHP_REFLECTABLES \
  HPHP_REFLECTABLES_UNQ\
  X(MixedArray::Elm)\
  /* */

#define X(name) struct name;
  HPHP_REFLECTABLES_UNQ
#undef X

///////////////////////////////////////////////////////////////////////////////

/*
 * Initialize the module by dynamically linking in generated code.
 *
 * Returns true on success, else false.
 */
bool init_member_reflection();

/*
 * Given an object pointer `base' and an internal pointer `internal' for
 * `base', get the name of the data member referred to by `internal'.
 *
 * Returns nullptr if reflection is not supported for the base type, or if the
 * internal pointer is invalid.
 */
#define X(name) \
  const char* nameof_member(const HPHP::name* base, const void* internal);
  HPHP_REFLECTABLES
#undef X

  template <typename T>
  const char* nameof_member(const T* /*base*/, const void* /*internal*/) {
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

namespace detail {

constexpr const char* const kMemberReflectionTableName =
  "g_member_reflection_vtable_impl";

}

///////////////////////////////////////////////////////////////////////////////

}

#endif
