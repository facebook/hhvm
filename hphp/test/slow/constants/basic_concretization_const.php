<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

abstract class A {
  abstract const int X = 3;
}

class C extends A {}

<<__EntryPoint>>
function main(): void {
  var_dump(C::X);
}
