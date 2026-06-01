<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function takes_representable_as_vec_int(\HH\Runtime\RepresentableAs<vec<int>> $c): void {}
function takes_representable_as_vec_num(\HH\Runtime\RepresentableAs<vec<num>> $c): void {}
function takes_representable_as_vec_arraykey(\HH\Runtime\RepresentableAs<vec<arraykey>> $c): void {}

function test_tuple_int_int((int, int) $t): void {
  // (int, int) <: RepresentableAs<vec<int>> — should pass
  takes_representable_as_vec_int($t);
}

function test_tuple_int_float((int, float) $t): void {
  // (int, float) <: RepresentableAs<vec<num>> — should pass (int|float <: num)
  takes_representable_as_vec_num($t);
}

function test_tuple_int_string((int, string) $t): void {
  // (int, string) <: RepresentableAs<vec<arraykey>> — should pass (int|string <: arraykey)
  takes_representable_as_vec_arraykey($t);
}

function test_tuple_int_string_not_vec_int((int, string) $t): void {
  // (int, string) <: RepresentableAs<vec<int>> — should FAIL
  takes_representable_as_vec_int($t);
}

// -- Traversable / KeyedTraversable tests --
// tuple upcast produces vec<T1 | T2 | ...>, which implements
// Traversable<T> and KeyedTraversable<int, T>.

function takes_representable_as_traversable_int(\HH\Runtime\RepresentableAs<Traversable<int>> $c): void {}
function takes_representable_as_traversable_num(\HH\Runtime\RepresentableAs<Traversable<num>> $c): void {}
function takes_representable_as_keyed_traversable_int_int(\HH\Runtime\RepresentableAs<KeyedTraversable<int, int>> $c): void {}
function takes_representable_as_keyed_traversable_int_num(\HH\Runtime\RepresentableAs<KeyedTraversable<int, num>> $c): void {}

function test_tuple_traversable_int((int, int) $t): void {
  // vec<int> <: Traversable<int> — should pass
  takes_representable_as_traversable_int($t);
}

function test_tuple_traversable_num((int, float) $t): void {
  // vec<int|float> <: Traversable<num> — should pass
  takes_representable_as_traversable_num($t);
}

function test_tuple_keyed_traversable_int_int((int, int) $t): void {
  // vec<int> <: KeyedTraversable<int, int> — should pass
  takes_representable_as_keyed_traversable_int_int($t);
}

function test_tuple_keyed_traversable_int_num((int, float) $t): void {
  // vec<int|float> <: KeyedTraversable<int, num> — should pass
  takes_representable_as_keyed_traversable_int_num($t);
}

function test_tuple_traversable_int_fails((int, string) $t): void {
  // vec<int|string> <: Traversable<int> — should FAIL
  takes_representable_as_traversable_int($t);
}

function test_tuple_keyed_traversable_int_int_fails((int, string) $t): void {
  // vec<int|string> <: KeyedTraversable<int, int> — should FAIL
  takes_representable_as_keyed_traversable_int_int($t);
}
