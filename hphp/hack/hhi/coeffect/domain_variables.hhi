<?hh
/**
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// Effect domain: variables (goal: simplify Rx/Pure type-checking)

namespace HH\Capabilities
 {
  /** A capability that allows untracked ownership & mutability, i.e.,
   * to avoid the ownership & mutability rules that are enforced in
   * certain contexts such as in all rx and rx-pure, or in const classes.
   */
  <<__Sealed(\HH\Capabilities\Defaults::class)>>
  interface UntrackOwnAndMut {}

  <<__Sealed(\HH\Capabilities\Defaults::class)>>
  interface Globals {}
}
