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
  <<__Sealed(Cipp::class)>>
  interface CippGlobal extends Server {}

  <<__Sealed()>>
  interface Cipp<T> extends CippGlobal {}
}

namespace HH\Contexts {
  type cipp_global = (\HH\Capabilities\CippGlobal & \HH\Capabilities\IO & \HH\Capabilities\Globals);

  // type cipp<T> = (\HH\Capabilities\Cipp<T> & cipp_global);
  type cipp<T> = (\HH\Capabilities\Cipp<T> & \HH\Capabilities\IO & \HH\Capabilities\Globals);

  namespace Unsafe {
    type cipp_global = mixed;
    type cipp<T> = mixed;
  }
}
