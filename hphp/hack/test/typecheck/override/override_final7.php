<?hh
/**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

abstract class P1 {
  final public function getTargetFBID(): int {
    return 0;
  }
}

abstract class P2 extends P1 {}

trait T1 {
  final public function getTargetFBID(): int {
    return 0;
  }
}

final class C1 extends P2 {
  use T1;
}
