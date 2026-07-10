<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

interface NoTB1 {}
interface NoTB2 {}

abstract class C {
  abstract const type TA as NoTB1 as NoTB2;

  // Neither upper bound declares TB, so `this::TA::TB` is an error.
  public function f(this::TA::TB $_): void {}
}
