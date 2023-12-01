<?hh

/*
 * It is permissible to drop optional fields and include additional fields in a
 * shape being used to fulfill a parameter that is a shape with optional and
 * unknown fields.
 */

type ShapeWithKnownAndUnknownFields = shape(
  'a' => int,
  ?'b' => string,
  ?'c' => bool,
  ...
);

function foo(ShapeWithKnownAndUnknownFields $argument): void {}

function bar(): void {
  foo(shape('a' => 42, 'b' => "b string", 'd' => 9000.1, 'e' => vec[]));
}
