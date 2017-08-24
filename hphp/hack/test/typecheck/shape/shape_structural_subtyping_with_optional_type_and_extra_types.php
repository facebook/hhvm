<?hh // strict

// The unprovided 'a' should not produce errors.
// The provided 'b' should not produce errors.
// The extra 'c' should not produce errors.
type ShapeWithOptionalField = shape(
  ?'a' => int,
  ?'b' => int,
  ...
);

function foo(ShapeWithOptionalField $argument): void {}

function bar(): void {
  foo(shape('a' => 42, 'c' => "Hello"));
}
