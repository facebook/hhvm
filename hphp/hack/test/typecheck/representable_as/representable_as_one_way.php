<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function takes_int(int $i): void {}
function takes_mixed(mixed $m): void {}

function test_representable_as_not_subtype_of_t(\HH\Runtime\RepresentableAs<int> $ci): void {
  // RepresentableAs<int> <: int — should FAIL
  takes_int($ci);
}

function test_representable_as_is_subtype_of_mixed(\HH\Runtime\RepresentableAs<int> $ci): void {
  // RepresentableAs<int> <: mixed — should pass (bound is mixed)
  takes_mixed($ci);
}
