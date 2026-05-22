<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function take_vec_int_repr(\HH\Runtime\RepresentableAs<vec<int>> $x): void {}

// A generic parameter bounded by a tuple type (int, int) should flow into
// a RepresentableAs<vec<int>> position via its bound: the tuple LHS rule
// reduces (int, int) <: RepresentableAs<vec<int>> to vec<int> <: vec<int>.
function generic_tuple<T as (int, int)>(T $val): void {
  take_vec_int_repr($val);
}
