<?hh // strict

// The provided 'a' should not produce errors.
// The unprovided 'b' should not produce errors.
// The extra 'c' should not produce errors.
type ShapeWithNullableFields = shape(
  'a' => ?int,
  'b' => ?int,
);

function foo(ShapeWithNullableFields $argument): void {}

function bar(): void {
  foo(shape('a' => 42, 'c' => "Hello"));
}
