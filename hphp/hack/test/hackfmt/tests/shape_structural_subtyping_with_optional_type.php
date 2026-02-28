<?hh

// Neither the unprovided 'a' nor the unprovided 'b' should produce errors.
type ShapeWithOptionalField = shape(
?'a' => int,
?'b' => int,
);

function foo(ShapeWithOptionalField $argument): void {}

function bar(): void {
  foo(shape());
}
