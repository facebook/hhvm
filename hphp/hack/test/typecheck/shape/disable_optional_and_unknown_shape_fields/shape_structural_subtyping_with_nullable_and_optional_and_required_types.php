<?hh // strict

// Only the unprovided 'b' should produce an error.
// The unprovided 'a' should not produce an error.
type ShapeWithOptionalNullableAndRequiredFields = shape(
  ?'a' => int,
  'b' => int,
  'c' => ?int,
);

function foo(ShapeWithOptionalNullableAndRequiredFields $argument): void {}

function bar(): void {
  foo(shape());
}
