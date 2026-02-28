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

trait T {
  final public static function f(): void {}
}

class ParentClass {
  use T;
}

class ChildClass extends ParentClass {
  use T;
}
