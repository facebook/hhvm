<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class F {
  /* HH_FIXME[4110] */
  public function instmeth<T as B>(
    classname<T> $classname,
  ): T {
  }
}

/* HH_FIXME[4110] */
function toplevel<T as B>(classname<T> $classname): T {
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
