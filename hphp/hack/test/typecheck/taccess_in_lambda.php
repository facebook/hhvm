<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function call<T>((function(): T) $f): T {
  return $f();
}

abstract class A {
  abstract const type T as arraykey;

  abstract public function f(): this::T;

  public function test(): arraykey {
    $x = call(() ==> $this->f());
    return $x;
  }
}

function test(A $a): arraykey {
  $x = call(() ==> $a->f());
  return $x;
}
