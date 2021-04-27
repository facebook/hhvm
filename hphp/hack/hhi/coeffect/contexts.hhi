<?hh
/**
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * Contexts and Capabilities:
 * See full documentation at
 * https://docs.hhvm.com/hack/contexts-and-capabilities/introduction
 *
 * Tl;dr, this framework allows a function to declare that the function
 * "does something," so that other functions can be statically understood
 * to do something (or not). The easiest example here is a side effect
 * like writing to a member variable:
 *
 * public function setFoo(Foo $foo)[write_props]: this {
 *   $this->foo = $foo;
 *   return $this;
 * }
 *
 * This simple setter method declares that it has a side effect, which is
 * to [write_props] which means "write to properties" which means that it
 * may alter the member variables.
 *
 * public function getFoo()[]: Foo {
 *   return $this->foo;
 * }
 *
 * This is declared to have no coeffects / side-effects; it doesn't do
 * anything notable.
 *
 * public function getFoo()[]: Foo {
 *   if ($this->foo is null) {
 *     // set the default
 *     $this->foo = Foo::getDefault();
 *   }
 *   return $this->foo;
 * }
 *
 * This is a hack error, because it writes to a property, but claims not to.
 *
 * Why do we care about this? Because someone else, far away in the codebase,
 * might call a getter and not realize that they're mutating the member.
 */

/**
 * To concisely model multiple capabilities, we use intersection types
 * (denoted by &). This is an exact approximation, unlike introducing
 * a new subtype via interface hierarchies and using multiple inheritance.
 * (In case of the latter, introducing all parent capabilities separately
 * would not be equal to introducing them via the subtype capability.)
 */
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

/**
* This namespace provides a mapping between user-facing contexts
* and a set of capabilities that determine the operations allowed (semantics).
* Contexts have snake-case name and appear in user code (syntax).
* Capabilities are modeled as a series of sealed interfaces:
* - extends clause that lists all other capabilities that are subsumed,
*   i.e., automatically present with the subtyped capability
*/
namespace HH\Contexts {

  /**
   * The default, normally unannotated context. This is currently hardcoded in
   * Typing_make_type.default_capability for performance reasons. The alias is
   * still present so that it may be directly used as [defaults]
   */
  type defaults = nothing; // an infinite set of all capabilities

  // TODO(coeffects) after implementing lower bounds on const ctx/type, do:
  /* = (
    \HH\Capabilities\WriteProperty &
    \HH\Capabilities\AccessGlobals &
    \HH\Capabilities\IO
  ); */

  /**
   * Described at the top of this file; this indicates that a method could
   * mutate a member variable ("write" a "property").
   */
  type write_props = \HH\Capabilities\WriteProperty;

  // TODO(cipp): deal with not giving it WriteProperty (or some other mechanism of turning on IFC)

  type policied = (
    \HH\Capabilities\ImplicitPolicy &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\IO &
    \HH\Capabilities\WriteProperty
  );
  // type policied_shallow = (\HH\Capabilities\ImplicitPolicyShallow & policied);
  type policied_shallow = (
    \HH\Capabilities\ImplicitPolicyShallow &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\IO &
    \HH\Capabilities\WriteProperty
  );
  // type policied_local = (\HH\Capabilities\ImplicitPolicyLocal & policied_shallow);
  type policied_local = (
    \HH\Capabilities\ImplicitPolicyLocal &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\IO &
    \HH\Capabilities\WriteProperty
  );
  // type policied_of<T> = (\HH\Capabilities\ImplicitPolicyOf<T> & policied);
  type policied_of<T> = (
    \HH\Capabilities\ImplicitPolicyOf<T> &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\IO &
    \HH\Capabilities\WriteProperty
  );
  // type policied_of_shallow<T> = (\HH\Capabilities\ImplicitPolicyOfShallow<T> & policied_of<T>);
  type policied_of_shallow<T> = (
    \HH\Capabilities\ImplicitPolicyOfShallow<T> &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\IO &
    \HH\Capabilities\WriteProperty
  );
  // type policied_of_local<T> = (\HH\Capabilities\ImplicitPolicyOfLocal<T> & policied_of_shallow<T>);
  type policied_of_local<T> = (
    \HH\Capabilities\ImplicitPolicyOfLocal<T> &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\IO &
    \HH\Capabilities\WriteProperty
  );

  type read_globals = \HH\Capabilities\ReadGlobals;
  type globals = \HH\Capabilities\AccessGlobals;

  type rx = (\HH\Capabilities\Rx & \HH\Capabilities\WriteProperty);
  // type rx_shallow = (\HH\Capabilities\RxShallow & rx);
  type rx_shallow = (\HH\Capabilities\RxShallow & \HH\Capabilities\WriteProperty);
  // type rx_local = (\HH\Capabilities\RxLocal & rx_shallow);
  type rx_local = (\HH\Capabilities\RxLocal & \HH\Capabilities\WriteProperty);
}
