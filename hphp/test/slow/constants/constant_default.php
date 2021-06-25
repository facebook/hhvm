<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

abstract class A {
  abstract const int X = 3;
}

interface I {
  const int X = 4;
}

class C extends A implements I {}

<<__EntryPoint>>
function main(): void {
  echo C::X;
}
