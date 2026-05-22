//// types.php
<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

newtype IntIntTuple as (int, int) = (int, int);

//// caller.php
<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function take_vec_int_repr(\HH\Runtime\RepresentableAs<vec<int>> $x): void {}

// A newtype bounded by a tuple type (int, int) should flow into a
// RepresentableAs<vec<int>> position via its bound. With IntIntTuple opaque
// outside its definition file, the Tnewtype LHS arm walks the bound chain
// so the tuple LHS arm of the rule reduces (int, int) <: RepresentableAs<vec<int>>
// to vec<int> <: vec<int>.
function test_newtype_tuple(IntIntTuple $val): void {
  take_vec_int_repr($val);
}
