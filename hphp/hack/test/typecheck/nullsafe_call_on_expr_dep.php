//// file1.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  public function f(): ?this {
    return $this;
  }
}

abstract class B extends A {
  public function g(): void {}
}

function b(): B {
  throw new Exception();
}

//// file2.php
<?hh

function call<Ta, Tb>(
  Ta $x,
  (function(Ta): Tb) $f,
): Tb {
  return $f($x);
}

function test(): void {
  $b = call(b(), $x ==> $x->f());
  call($b, $x ==> $x?->g());
}
