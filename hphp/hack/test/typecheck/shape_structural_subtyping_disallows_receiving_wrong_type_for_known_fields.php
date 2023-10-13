<?hh

/*
 * A parameter that is a shape with unknown fields should not be fulfilled by a
 * shape that has a different type for one of the known fields.
 */

type ShapeWithKnownAndUnknownFields = shape(
  'a' => int,
  ...
);

function foo(ShapeWithKnownAndUnknownFields $argument): void {}

function bar(): void {
  foo(shape('a' => "a string"));
}
