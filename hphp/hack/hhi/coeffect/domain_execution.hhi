<?hh
/**
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// Effect domain: Execution

namespace HH\Capabilities {
  /**
   * The capability for non-determinism
   */
  <<__Sealed(\HH\Contexts\non_det::class)>>
  interface NonDet {}
}
