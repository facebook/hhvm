<?hh

abstract class C {
  const type TD = D;
}

abstract class D {
  const type TE = E;
}

abstract class E {
  abstract const type T as arraykey;
}

function f(mixed $x): void {
  if ($x is C::TD::TE::T) {
    expect_arraykey($x);
  }
}

function expect_arraykey(arraykey $x): void {}
