<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class A { public function g(num $_): void {} }
class B extends A {
    <<__Override>>
  public function g<T>(T $_): void where num as T {}
}
