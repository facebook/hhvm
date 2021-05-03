<?hh
/**
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

namespace HH\Capabilities {
  <<__Sealed>>
  interface Unrelated {}
}

namespace HH\Contexts {
  type oldrx = (\HH\Capabilities\Rx & \HH\Capabilities\WriteProperty);
  type oldrx_shallow = (\HH\Capabilities\RxShallow & \HH\Capabilities\WriteProperty);
  type oldrx_local = (\HH\Capabilities\RxLocal & \HH\Capabilities\WriteProperty);

  type unrelated = \HH\Capabilities\Unrelated;

  namespace Unsafe {
    type oldrx = mixed;
    type oldrx_shallow = \HH\Capabilities\RxLocal;
    type oldrx_local = \HH\Contexts\defaults;

    type unrelated = mixed;
  }
}
