<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class Foo {
  public function getAnon1(): (function(int): this) {
    return function(int $x): this {
      return $this;
    };
  }

  public function getAnon2(): (function(int): this) {
    return function(int $x) {
      return $this;
    };
  }
}

function test1(Foo $f): Foo {
  $anon = $f->getAnon1();
  return $anon(123);
}

function test2(Foo $f): Foo {
  $anon = $f->getAnon2();
  return $anon(123);
}
