<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function take_int_repr(\HH\Runtime\RepresentableAs<int> $x): void {}

// A dependent type (this::T) whose declared bound is RepresentableAs<int>
// should flow into a RepresentableAs<int> position via its bound: the
// Tdependent LHS arm walks the bound chain so RepresentableAs<int>
// satisfies the rhs RepresentableAs<int>.
abstract class HasIntRepr {
  abstract const type T as \HH\Runtime\RepresentableAs<int>;

  public function test(this::T $val): void {
    take_int_repr($val);
  }
}
