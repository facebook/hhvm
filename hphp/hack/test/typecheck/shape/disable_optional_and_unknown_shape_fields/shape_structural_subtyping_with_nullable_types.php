<?hh // strict

// Neither the unprovided 'a' nor the unprovided 'b' should produce errors.
type ShapeWithNullableFields = shape(
  'a' => ?int,
  'b' => ?int,
);

function foo(ShapeWithNullableFields $argument): void {}

function bar(): void {
  foo(shape());
}
