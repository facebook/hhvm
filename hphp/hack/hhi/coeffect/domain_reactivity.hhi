<?hh
/**
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

namespace HH\Capabilities {
  /**
   * The core capability present in every reactive context.
   * Each weaker level of reactive context has additional privileges,
   * thus the respective capabilities are subtypes of this one.
   */
  <<__Sealed(RxShallow::class)>>
  interface Rx extends Server {}

  <<__Sealed(RxLocal::class)>>
  interface RxShallow extends Rx {}

  <<__Sealed()>>
  interface RxLocal extends RxShallow {}
}

namespace HH\Contexts {
  type rx = (\HH\Capabilities\Rx & \HH\Capabilities\IO);

  // type rx_shallow = (\HH\Capabilities\RxShallow & rx);
  type rx_shallow = (\HH\Capabilities\RxShallow & \HH\Capabilities\IO);

  // type rx_local = (\HH\Capabilities\RxLocal & rx_shallow);
  type rx_local = (\HH\Capabilities\RxLocal & \HH\Capabilities\IO);

  namespace Unsafe {
    type rx = mixed;
    type rx_shallow = \HH\Capabilities\RxLocal;
    type rx_local = \HH\Contexts\defaults;
  }
}
