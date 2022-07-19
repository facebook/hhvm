<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

interface I {
  abstract const int X = 3;
  abstract const type T = int;
  abstract const ctx C = [];
}

abstract final class B implements I {}
