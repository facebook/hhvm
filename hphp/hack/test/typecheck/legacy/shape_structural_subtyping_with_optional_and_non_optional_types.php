<?hh // strict

// Only the unprovided 'b' should produce an error.
// The unprovided 'a' should not produce an error.
type ShapeWithOptionalAndNonOptionalFields = shape(
  ?'a' => int,
  'b' => int,
);

function foo(ShapeWithOptionalAndNonOptionalFields $argument): void {}

function bar(): void {
  foo(shape());
}
