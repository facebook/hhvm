<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type Scalar = bool | int | float;

function expect_scalar(Scalar $scalar): void {
}

function f<T as Scalar>(T $scalar): void {
  expect_scalar($scalar);
}
