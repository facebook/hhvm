<?hh
/**
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// Effect domain: Reactivity

namespace HH\Capabilities {
  /**
   * The core capability present in every reactive context.
   * Each weaker level of reactive context has additional privileges,
   * thus the respective capabilities are subtypes of this one.
   */
  <<__Sealed(RxShallow::class, \HH\Contexts\rx::class)>>
  interface Rx {} // long-term: extends Throwing<mixed>

  <<__Sealed(RxLocal::class, \HH\Contexts\rx_shallow::class)>>
  interface RxShallow extends Rx {}

  <<__Sealed(Defaults::class, \HH\Contexts\rx_local::class, \HH\Contexts\rx_shallow::class)>>
  interface RxLocal extends RxShallow {}
}
