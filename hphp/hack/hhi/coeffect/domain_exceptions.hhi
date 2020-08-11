<?hh
/**
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// Effect domain: exceptions (EXPERIMENTAL)

namespace HH\Capabilities {
  /**
   * Throwing a wider set of exceptions (i.e., supertypes of T)
   * requires more privilege, therefore the capability to throw is
   * contravariant in T.  E.g., if a function is privileged to throw
   * `MyException` that is a subtype of \Throwable, it should not be able
   * to escalate its privilege via upcasting to `Throwing<\Throwable>`
   * and effectively call another funcion that can throw anything
   * (i.e., that requires a `Throwing<\Throwable>` capability).
   */
  <<__Sealed(Rx::class)>>
  interface Throwing<-T> {}
}
