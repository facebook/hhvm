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


class Foo {
  final protected function f1(): void {
  }
}
class Bar extends Foo {
  protected function f1(): void {
  }
}
