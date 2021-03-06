<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

abstract class A {
  abstract const int X = 3;
}

trait T {
  const int X = 5;
}

class C extends A {
  use T;
}

<<__EntryPoint>>
function main(): void {
  echo C::X;
}
