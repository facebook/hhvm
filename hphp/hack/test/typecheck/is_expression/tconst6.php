<?hh // strict

abstract class C {
  const type TD = D;
}

abstract class D {
  const type TE = E;
}

abstract class E {
  const type T = vec<int>;
}

function f(mixed $x): void {
  if ($x is C::TD::TE::T) {
    expect_vec_int($x);
  }
}

function expect_vec_int(vec<int> $x): void {}
