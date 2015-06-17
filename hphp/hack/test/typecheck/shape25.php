<?hh

/**
 * Structural subtyping of ad-hoc shapes in presence of optional fields
 */
type t = shape(
  'x' => int,
  'z' => ?bool,
);

// No error: we are sure that there is no field 'z' in returned shape
function test(s $s): t {
  return shape('x' => 4);
}
