<?hh

// Regression test for shape-field error positions, rendered with the
// `--error-format plain_highlighted` format (see the HH_FLAGS in this directory).
//
// When a shape literal has a field whose value doesn't match the expected
// shape type, the error should point at the *specific* offending field, not at
// the entire shape expression.

type TShape = shape(
  'a' => int,
  'b' => string,
  'c' => int,
);

// Wrong field in a returned shape literal: the error should point at field 'b'.
function test_return(): TShape {
  return shape(
    'a' => 1,
    'b' => 3,
    'c' => 2,
  );
}
