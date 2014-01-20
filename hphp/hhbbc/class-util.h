/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_CLASS_UTIL_H_
#define incl_HPHP_CLASS_UTIL_H_

#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {

namespace res { struct Class; }
namespace php { struct Class; }
struct Type;

//////////////////////////////////////////////////////////////////////

/*
 * Returns whether a res::Class refers to a collection class.
 */
bool is_collection(res::Class);

/*
 * Returns whether a Type could hold an object that has a custom
 * boolean conversion function.
 */
bool could_have_magic_bool_conversion(Type);

/*
 * A mask of which types of special functions a class has.
 */
enum class MethodMask : uint32_t {
  Internal_86pinit = 0x1,
  Internal_86sinit = 0x2,
};
inline bool contains(MethodMask mask, MethodMask val) {
  return static_cast<uint32_t>(mask) & static_cast<uint32_t>(val);
}

/*
 * Returns a mask that indicates special methods a class has.
 */
MethodMask find_special_methods(borrowed_ptr<const php::Class>);

/*
 * Returns a collection type name given a Collection::Type.
 */
SString collectionTypeToString(uint32_t ctype);

//////////////////////////////////////////////////////////////////////

}}

#endif
