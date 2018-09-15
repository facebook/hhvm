<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class UnrelatedParent {}

class CParent {
  public function foo(): void {}
}

trait ATrait {
  public function foo(): void {}
}

trait UnrelatedTrait {}

interface I {
  public function bar(): void;
}

class CTrait extends UnrelatedParent implements I {
  use ATrait;

  <<__Override>>
  public function foo(): void {}

  public function bar(): void {}
}

class CParentAndTrait extends CParent implements I {
  use ATrait;
  use Unrelated;

  <<__Override>>
  public function foo(): void {}

  public function bar(): void {}
}
