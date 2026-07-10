<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

interface HasTB {
  abstract const type TB as arraykey;
}
interface HasTB2 {
  abstract const type TB as int;
}
interface NoTB {}

function takes_arraykey(arraykey $_): void {}
function takes_int(int $_): void {}

abstract class OneBound {
  // Only one of the two upper bounds declares TB. Nested access must still
  // resolve, since `TA <: (HasTB & NoTB)` and `HasTB` provides `TB`.
  abstract const type TA as HasTB as NoTB;

  public function f(this::TA::TB $tb): void {
    takes_arraykey($tb);
  }
}

abstract class BothBounds {
  // Both upper bounds declare TB; nested access sees the intersection of the
  // two declared bounds, so `TB <: int`.
  abstract const type TA as HasTB as HasTB2;

  public function f(this::TA::TB $tb): void {
    takes_arraykey($tb);
    takes_int($tb);
  }
}
