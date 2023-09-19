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

#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/attr.h"

#include "hphp/hhbbc/misc.h"

namespace HPHP {

struct UserAttributeMap;

namespace HHBBC {

namespace res { struct Class; }
namespace php {
struct Class;
struct Func;
struct Prop;
}
struct Index;
struct Type;

//////////////////////////////////////////////////////////////////////

/*
 * Returns whether a res::Class refers to a collection class.
 */
bool is_collection(res::Class);

/*
 * Returns whether a php::Class is the base class for all closures.
 */
bool is_closure_base(const php::Class&);
bool is_closure_base(SString);

/*
 * Returns whether a php::Class is a closure.
 */
bool is_closure(const php::Class&);

/*
 * Whether the given name is that of a closure.
 */
bool is_closure_name(SString);

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
 * Whether a method by this name should get a "name-only" func family
 * entry.
 */
bool has_name_only_func_family(SString);

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
 * Returns true if the given class is "regular". That is, not an
 * interface, enum, trait, or abstract class. Those types of classes
 * cannot be instantiated (but can be interacted with statically).
 */
bool is_regular_class(const php::Class&);
bool is_regular_class(Attr);

Type get_type_of_reified_list(const UserAttributeMap& ua);

TypedValue get_default_value_of_reified_list(const UserAttributeMap& ua);

/*
 * Given a type representing a prop with name "name" on the class
 * "ctx", loosen the type appropriately to represent any possible
 * deserializing of the prop. Generally, this discards any special
 * inferred array structure or known values.
 */
Type loosen_this_prop_for_serialization(const php::Class& ctx,
                                        SString name,
                                        Type type);

//////////////////////////////////////////////////////////////////////

}}
