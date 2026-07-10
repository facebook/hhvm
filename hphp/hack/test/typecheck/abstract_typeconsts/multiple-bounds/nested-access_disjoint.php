<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

interface HasTBInt {
  abstract const type TB as int;
}
interface HasTBString {
  abstract const type TB as string;
}

function takes_int(int $_): void {}
function takes_string(string $_): void {}

abstract class C {
  // Both upper bounds declare TB, but with disjoint bounds: the nested access
  // `this::TA::TB` has upper bound `int & string`, which is `nothing`. The value
  // is therefore usable at both `int` and `string` (nothing is a subtype of
  // every type). The type constant is uninhabitable but well-formed.
  abstract const type TA as HasTBInt as HasTBString;

  public function f(this::TA::TB $tb): void {
    takes_int($tb);
    takes_string($tb);
  }
}
