<?hh // strict
/**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class ParentClass {
  final public function f(): void {}
}

trait T {
  require extends ParentClass;
}

class ChildClass extends ParentClass {
  use T;
}
