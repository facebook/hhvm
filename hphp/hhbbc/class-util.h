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
#ifndef incl_HHBBC_CLASS_UTIL_H_
#define incl_HHBBC_CLASS_UTIL_H_

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"

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
 * Returns whether a php::Class is a closure.
 */
bool is_closure(const php::Class&);

/*
 * Returns whether a Type could hold an object that has a custom
 * boolean conversion function.
 */
bool could_have_magic_bool_conversion(Type);

/*
 * Returns method named "name" if it exists.
 */
borrowed_ptr<php::Func> find_method(borrowed_ptr<const php::Class>,
                                    SString name);

/*
 * Returns true if `name' is the name of an internal VM special class
 * method.  (Not callable directly by php code.)
 */
bool is_special_method_name(SString name);

/*
 * Returns true if a class has the __MockClass user attribute.  This
 * attribute allows final methods and final classes to be overridden.
 */
bool is_mock_class(borrowed_ptr<const php::Class>);

//////////////////////////////////////////////////////////////////////

}}

#endif
