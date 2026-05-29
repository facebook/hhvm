<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function takes_representable_as_num(\HH\Runtime\RepresentableAs<num> $c): void {}
function takes_representable_as_int(\HH\Runtime\RepresentableAs<int> $c): void {}

function test_covariant(\HH\Runtime\RepresentableAs<int> $ci): void {
  // RepresentableAs<int> <: RepresentableAs<num> — should pass
  takes_representable_as_num($ci);
}

function test_not_contravariant(\HH\Runtime\RepresentableAs<num> $cn): void {
  // RepresentableAs<num> <: RepresentableAs<int> — should FAIL
  takes_representable_as_int($cn);
}
