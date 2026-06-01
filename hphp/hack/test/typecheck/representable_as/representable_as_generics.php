<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function wrap_in_representable_as<T>(T $x): \HH\Runtime\RepresentableAs<T> {
  // T <: RepresentableAs<T> — should pass
  return $x;
}

function try_unwrap_representable_as<T>(\HH\Runtime\RepresentableAs<T> $c): T {
  // RepresentableAs<T> <: T — should FAIL
  return $c;
}
