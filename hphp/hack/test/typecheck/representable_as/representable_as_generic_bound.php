<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function take_int_repr(\HH\Runtime\RepresentableAs<int> $x): void {}
function take_string_repr(\HH\Runtime\RepresentableAs<string> $x): void {}

// A generic parameter bounded by RepresentableAs<int> should flow into
// a RepresentableAs<int> position via its bound.
function generic_int<T as \HH\Runtime\RepresentableAs<int>>(T $val): void {
  take_int_repr($val);
}

// Same for RepresentableAs<string>.
function generic_string<T as \HH\Runtime\RepresentableAs<string>>(T $val): void {
  take_string_repr($val);
}
