<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

interface I1 {
  abstract const int X = 3;
}

interface I2 {
  abstract const arraykey X;
}

abstract class A {
  abstract const num X;
}

class C extends A implements I1, I2 {}

function f(): void {
  $x = C::X;
  hh_show($x);
}
