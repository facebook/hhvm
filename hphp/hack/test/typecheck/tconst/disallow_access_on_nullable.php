<?hh // strict

class X {
  const type A = ?X;
  const type B = X;
  const type C = X::A::B;
}
