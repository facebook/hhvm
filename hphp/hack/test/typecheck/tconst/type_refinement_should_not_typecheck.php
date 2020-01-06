<?hh // strict

abstract class C2 {
  abstract const type T2;
}

class D0 {}
class D1 {}

class A0 extends C2 { const type T2 = D0; }
class A1 extends C2 { const type T2 = D1; }

interface B {
  abstract const type T1 as C2;
  public function fn1(this::T1::T2 $x): void;
}
interface B0 extends B { abstract const type T1 as A0; }
interface B1 extends B { abstract const type T1 as A1; }

function test(mixed $arg, B $x): void {
  if ($x is B0) {
    $x->fn1($arg as D0);
  } else if ($x is B1) {
    // hh_show_env();
    $x->fn1($arg as D1);
    // FIXME: Knowledge from the branch above leaks
    // in this branch. See T59317869.
    $x->fn1($arg as D0); // should not typecheck
  }
}
