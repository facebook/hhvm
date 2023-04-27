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
 * # Contexts and Capabilities
 *
 * See full documentation at
 * https://docs.hhvm.com/hack/contexts-and-capabilities/introduction
 *
 * Capabilities describe what a function in Hack is allowed to do.
 *
 * Capabilities are grouped into sets called contexts. You can mark your
 * functions with contexts, and Hack will ensure that you have all the
 * capabilities required.
 *
 * ```
 * function birthday_good(Person $p)[write_props]: void {
 *   $p->age += 1;
 * }
 *
 * function birthday_bad(Person $p)[]: void {
 *   // Type checker error: this function isn't allowed to write properties.
 *   $p->age += 1;
 * }
 * ```
 *
 * `birthday_good` has the `write_props` context, but `birthday_bad` has
 * the pure context (no capabilities). The type checker is checking that
 * `birthday_bad` is a pure function.
 *
 * A function may have more than one context.
 *
 * ```
 * function example()[globals, write_props]: void {
 *   // You can do things with static or instance properties here.
 * }
 * ```
 */

// We represent a context as an intersection type of all the
// capabilities it includes, so enable the `&` type syntax.
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

/**
 * This namespace defines all the contexts available in HHVM, the Hack
 * runtime. When your code runs, HHVM will check that your functions have
 * the necessary capabilities in their contexts.
 */
namespace HH\Contexts {

  /**
   * The default set of capabilities. If you don't specify a context on
   * your function, it's the same as writing `[defaults]`.
   *
   * `defaults` includes almost all capabilities, including:
   *
   * * writing to stdout (e.g. `echo $foo`)
   * * modifying instance properties (e.g. `$p->age += 1`)
   * * accessing static properties (e.g. `MySingleton::$activeInstance`)
   * * calling other functions that use any of these capabilities
   *
   * `defaults` functions may call into `zoned` functions, but not the
   * other way around.
   *
   * `defaults` also excludes the capabilities required to call (or be called
   * from) `zoned_with` functions.
   */
  type defaults = (
    \HH\Capabilities\WriteProperty &
    \HH\Capabilities\AccessGlobals &
    \HH\Capabilities\RxLocal &
    \HH\Capabilities\SystemLocal &
    \HH\Capabilities\ImplicitPolicyLocal &
    \HH\Capabilities\IO &
  );

  /**
   * The `write_props` context allows your function to write to instance
   * variables.
   *
   * `write_props` is not necessary on `__construct` methods that only
   * write to `$this`.
   */
  type write_props = \HH\Capabilities\WriteProperty;

  // TODO(cipp): deal with not giving it WriteProperty (or some other mechanism of turning on IFC)

  /**
   * The `leak_safe` context ensures that your function cannot leak data by
   * writing to global state.
   *
   * `leak_safe` functions can only return data by passing it as a return
   * value or writing it to arguments.
   */
  type leak_safe = (
    \HH\Capabilities\WriteProperty &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\System &
  );

  /**
   * The `leak_safe_shallow` context is a less restricted form of the
   * `leak_safe` context, to help with gradual migration to `leak_safe`.
   *
   * Functions with the `leak_shallow` context have the same
   * restrictions as `leak_safe` for internal operations (e.g. no mutating
   * static variables). They are also allowed to call `leak_safe_local` or
   * `leak_safe_shallow` functions.
   *
   * See also `leak_safe_local`.
   */
  type leak_safe_shallow = (
    \HH\Capabilities\WriteProperty &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\SystemShallow &
  );

  /**
   * The `leak_safe_local` context is the least restricted form of the
   * `leak_safe` context, to help with gradual migration to `leak_safe`.
   *
   * Functions with the `leak_safe_local` context have the same restrictions
   * as `leak_safe` for internal operations (e.g. no mutating static
   * variables). They are also allowed to call `leak_safe_local` or
   * `leak_safe_shallow` functions and their leak-safe counterparts,
   * and even `defaults` functions.  However, they cannot call
   * functions with the `zoned_with` context nor its shallow/local variant.
   */
  type leak_safe_local = (
    \HH\Capabilities\WriteProperty &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\SystemLocal &
  );

  /**
   * `zoned` includes all the capabilities of `leak_safe`, but also allows
   * accessing the current zone policy.
   *
   * `zoned` functions may be called from either `defaults` functions or
   * `zoned_with` functions. `zoned` is more restrictive, so it cannot call
   * into `defaults` or `zoned_with` functions.
   */
  type zoned = (
    \HH\Capabilities\WriteProperty &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\ImplicitPolicy &
    \HH\Capabilities\System &
  );

  /**
   * The `zoned_shallow` context is a less restricted form of the `zoned`
   * context, to help with gradual migration to `zoned`.
   *
   * Functions with the `zoned_shallow` context have the same
   * restrictions as `zoned` for internal operations (e.g. no mutating
   * static variables). They are also allowed to call `zoned_local` or
   * `zoned_shallow` functions, as well as `leak_safe_shallow` functions.
   *
   * See also `zoned_local`.
   */
  type zoned_shallow = (
    \HH\Capabilities\WriteProperty &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\ImplicitPolicyShallow &
    \HH\Capabilities\SystemShallow &
  );

  /**
   * The `zoned_local` context is the least restricted form of the
   * `zoned` context, to help with gradual migration to `zoned`.
   *
   * Functions with the `zoned_local` context have the same restrictions
   * as `zoned` for internal operations (e.g. no mutating static
   * variables). They are also allowed to call `zoned_local` or
   * `zoned_shallow` functions and their leak-safe counterparts,
   * and even `defaults` functions.  Notably, they cannot call
   * functions with the `zoned_with` context nor its shallow/local variant.
   */
  type zoned_local = (
    \HH\Capabilities\WriteProperty &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\ImplicitPolicyLocal &
    \HH\Capabilities\SystemLocal &
  );

  /**
   * The `zoned_with<Foo>` context allows your function to be used with
   * other `zoned_with<Foo>` functions.
   *
   * This is similar to `zoned`, but allows you define multiple distinct
   * zones.
   *
   * `zoned_with` functions require a special entry point, and cannot be
   * called from `defaults` functions.
   */
  type zoned_with<T> = (
    \HH\Capabilities\WriteProperty &
    \HH\Capabilities\ReadGlobals &
    \HH\Capabilities\ImplicitPolicyOf<T> &
    \HH\Capabilities\System &
  );

  /**
   * The `read_globals` context gives your function the capability to read
   * global state, such as static properties on classes.
   *
   * To avoid mutating static properties, functions using this context can
   * only access static properties using `readonly`.
   *
   * See also `globals`.
   */
  type read_globals = \HH\Capabilities\ReadGlobals;

  /**
   * The `globals` context gives your function the capability to read
   * and write global state, such as static properties on classes.
   *
   * See `read_globals` if you only need to read global state.
   */
  type globals = \HH\Capabilities\AccessGlobals;

  type rx = (\HH\Capabilities\Rx & \HH\Capabilities\WriteProperty);
  // type rx_shallow = (\HH\Capabilities\RxShallow & rx);
  type rx_shallow =
    (\HH\Capabilities\RxShallow & \HH\Capabilities\WriteProperty);
  // type rx_local = (\HH\Capabilities\RxLocal & rx_shallow);
  type rx_local = (\HH\Capabilities\RxLocal & \HH\Capabilities\WriteProperty);
}
