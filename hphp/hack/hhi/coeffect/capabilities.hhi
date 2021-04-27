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
 * This namespace contains pseudo-types that represent capabilities.
 * To establish calling conventions (i.e., which function/method can
 * be safely called from certain contexts), the typechecker desugars
 * each context into a set of capabilities in this namespace.
 *
 * (A set of) capabilities act as a permission for enforcing permitted
 * combination of caller/callee contexts while type-checking.
 * In layman's terms, an active capability can be converted to
 * any of its supertypes (but not subtypes), and a function/method
 * call type-checks if and only if:
 * - for every capability required by the callee, there is
 *   a matching capability present at the call site (i.e., caller),
 * where a capability of caller may be reused and converted to
 * any of its supertype capabilities multiple times in order to
 * match each capability required by the caller.
 *
 * From a formal standpoint, a set of capabilities is represented as
 * a single intersection type between the constituent capabilities,
 * and the only requirement is that the caller's capability type is
 * a subtype of the callee's capability type.
 */
namespace HH\Capabilities {

  <<__Sealed(AccessGlobals::class)>>
  interface ReadGlobals {}

  <<__Sealed()>>
  interface AccessGlobals extends ReadGlobals {}

  <<__Sealed()>>
  interface IO {}

  <<__Sealed()>>
  interface WriteProperty {}

  <<__Sealed(ImplicitPolicyOf::class, ImplicitPolicyShallow::class)>>
  interface ImplicitPolicy {}
  <<__Sealed(ImplicitPolicyOfShallow::class, ImplicitPolicyLocal::class)>>
  interface ImplicitPolicyShallow extends ImplicitPolicy {}
  <<__Sealed(ImplicitPolicyOfLocal::class)>>
  interface ImplicitPolicyLocal extends ImplicitPolicyShallow {}

  <<__Sealed(ImplicitPolicyOfShallow::class)>>
  interface ImplicitPolicyOf<T> extends ImplicitPolicy {}
  <<__Sealed(ImplicitPolicyOfLocal::class)>>
  interface ImplicitPolicyOfShallow<T> extends ImplicitPolicyOf<T>, ImplicitPolicyShallow {}
  <<__Sealed()>>
  interface ImplicitPolicyOfLocal<T> extends ImplicitPolicyOfShallow<T>, ImplicitPolicyLocal {}

  /**
   * The core capability present in every reactive context.
   * Each weaker level of reactive context has additional privileges,
   * thus the respective capabilities are subtypes of this one.
   */
  <<__Sealed(RxShallow::class)>>
  interface Rx {}

  <<__Sealed(RxLocal::class)>>
  interface RxShallow extends Rx {}

  <<__Sealed()>>
  interface RxLocal extends RxShallow {}
}
