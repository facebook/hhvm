<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

abstract class A {
  abstract const int X = 3;
}

interface I1 {
  const int X = 4;
}

interface I2 {
  abstract const int X = 5;
}

class C extends A implements I1, I2 {}

<<__EntryPoint>>
function main(): void {
  echo C::X;
}
