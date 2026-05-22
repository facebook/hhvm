<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function take_vec_int_repr(\HH\Runtime\RepresentableAs<vec<int>> $x): void {}

// A dependent type (this::T) whose declared bound is the tuple type
// (int, int) should flow into a RepresentableAs<vec<int>> position via
// its bound: the Tdependent LHS arm walks the bound chain so the tuple
// LHS arm of the rule reduces (int, int) <: RepresentableAs<vec<int>>
// to vec<int> <: vec<int>.
abstract class HasTupleBound {
  abstract const type T as (int, int);

  public function test(this::T $val): void {
    take_vec_int_repr($val);
  }
}
