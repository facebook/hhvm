<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

abstract class A {
  abstract const int X = 3;
  abstract const type T = int;
  abstract const ctx C = [];
}

abstract final class B extends A {}
