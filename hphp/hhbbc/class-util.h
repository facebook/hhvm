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
 * Returns whether a clsName is a class with a magic toBoolean method.
 */
bool has_magic_bool_conversion(SString clsName);

/*
 * Returns method named "name" if it exists.
 */
php::Func* find_method(const php::Class*, SString name);

/*
 * Returns true if `name' is the name of an internal VM special class
 * method.  (Not callable directly by php code.)
 */
bool is_special_method_name(SString name);

/*
 * Returns true if a class has the __MockClass user attribute.  This
 * attribute allows final methods and final classes to be overridden.
 */
bool is_mock_class(const php::Class*);

/*
 * Returns true if the given trait class has the __NoFlatten user attribute.
 * This can be used to forcibly disable flattening for the given trait.
 * Asserts that the class passed in is a trait.
 */
bool is_noflatten_trait(const php::Class*);

/*
 * Returns true if cls is a trait which will not be imported into any
 * classes at runtime (probably because it was flattened into them).
 */
bool is_unused_trait(const php::Class& cls);

/*
 * Returns true if cls is a trait which could be imported into a class
 * at runtime.
 */
bool is_used_trait(const php::Class& cls);

/*
 * Normalizes a class' name to remove any non-deterministic elements. For
 * closures and anonymous classes, it removes the unique integer identifier from
 * it. Otherwise, it returns the unmodified class name.
 */
std::string normalized_class_name(const php::Class& cls);

/*
 * Returns true if the property has an initial value which might
 * possibly violate its type-hint. If it returns false, it is
 * guaranteed to not violate the type-hint.
 */
bool prop_might_have_bad_initial_value(const Index& index,
                                       const php::Class& cls,
                                       const php::Prop& prop);

//////////////////////////////////////////////////////////////////////

}}
