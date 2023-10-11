<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class F {
  public function instmeth<T as B>(
    classname<T> $classname,
  ): T {
    throw new Exception();
  }
}

function toplevel<T as B>(classname<T> $classname): T {
  throw new Exception();
}

final class C extends B {
  public function foo(): void { }
}

abstract class B { }
function testIt(F $f):void {
  $x = toplevel(C::class);
  $x->foo();
  $y = $f->instmeth(C::class);
  $y->foo();
}
