<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

final class Foo {
  /* For a non-final class, new static() and new self() are quite different. For
   * a final class, they are the same */
  public function get1(): this {
    return new Foo();
  }

  public function get2(): this {
    return new self();
  }
}
