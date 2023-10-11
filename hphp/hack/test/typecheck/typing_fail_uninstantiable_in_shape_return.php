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

  const type TData = shape('text' => CannotBeInstantiated);
  // this type can never be satisfied
  public function foo(this::TData $c): void {}
}
