<?hh // strict
/**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class ParentClass {
  final public function f(): void {}
}

trait T {
  final public function f(): void {}
}

class ChildClass extends ParentClass {
  use T;
}
