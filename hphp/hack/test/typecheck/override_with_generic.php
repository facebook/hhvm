<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.


class Contra<-T> {}

class A { public function f(Contra<num> $t): void {} }

class B extends A {
    <<__Override>>
  public function f<T>(Contra<T> $_): void where T as num {}
}
