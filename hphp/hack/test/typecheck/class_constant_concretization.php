<?hh

abstract class A {
  abstract const int X = 4;
}

class C extends A {}

function f(): void {
  $x = C::X;
  hh_show($x);
  $fail = A::X;
}
