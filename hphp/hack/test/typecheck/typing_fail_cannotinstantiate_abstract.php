<?hh
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

abstract final class CannotBeInstantiated {}
class TestFoo {

  // this type can never be satisfied
  public function foo(CannotBeInstantiated $c): void {}
}
