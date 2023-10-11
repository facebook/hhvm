<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public int $x = 42;
}

class B {
  public string $x = 'answer';
}

function test(bool $c, A $a, B $b): arraykey {
  if ($c) {
    $ab = $a;
  } else {
    $ab = $b;
  }
  return $ab?->x;
}
