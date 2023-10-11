<?hh

/*
 * It is permissible to include additional fields when passing a shape into a
 * method that requires a shape that contains unknown fields.
 */

type ShapeWithKnownAndUnknownFields = shape(
  'a' => int,
  'b' => string,
  'c' => bool,
  ...
);

function foo(ShapeWithKnownAndUnknownFields $argument): void {}

function bar(): void {
  foo(
    shape(
      'a' => 42,
      'b' => "b string",
      'c' => true,
      'd' => 9000.1,
      'e' => varray[],
    ),
  );
}
