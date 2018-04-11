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
interface GenReadApi<Tk, +Tv> {}

class GenReadApiClass<Tk, +Tv> implements GenReadApi<Tk, Tv> {
  public function __construct(private Tv $v) {}
}

class Foo {
  public static function make<Tk>(): GenReadApi<Tk, mixed> {
    return new GenReadApiClass(null);
  }
}

final class Bar extends Foo {
  public static function make<Tk>(): GenReadApi<Tk, this> {
    return new GenReadApiClass(new static());
  }
}

interface GenReadIdxApi<Tk, +Tv> {}

class GenReadIdxApiClass<Tk, +Tv> implements GenReadIdxApi<Tk, Tv> {
  public function __construct(private Tv $v) {}
}

class FooIdx {
  public static function make<Tk>(): GenReadIdxApi<Tk, mixed> {
    return new GenReadIdxApiClass(null);
  }
}

<<__ConsistentConstruct>>
class BarIdx extends FooIdx {
  public static function make<Tk>(): GenReadIdxApi<Tk, this> {
    return new GenReadIdxApiClass(new static());
  }
}
