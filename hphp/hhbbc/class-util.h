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
 * Returns whether a php::Class contains an 86pinit method.
 */
bool has_86pinit(borrowed_ptr<const php::Class>);

/*
 * Returns a collection type name given a Collection::Type.
 */
SString collectionTypeToString(uint32_t ctype);

//////////////////////////////////////////////////////////////////////

}}

#endif
