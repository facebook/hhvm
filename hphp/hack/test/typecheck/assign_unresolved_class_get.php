<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static num $x = 3.14;
}

class B {
  public static arraykey $x = 'foo';
}

function test(bool $b): void {
  if ($b) {
    $c = A::class;
  } else {
    $c = B::class;
  }
  $c::$x = 42;
}
