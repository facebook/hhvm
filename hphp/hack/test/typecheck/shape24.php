<?hh

/**
 * Structural subtyping of declared shapes in presence of optional fields
 */

type s = shape('x' => int);

type t = shape(
  'x' => int,
  'z' => ?bool,
);

// Error: s declares only 'x', but at runtime it can also have a 'z' that
// we don't know anything about. 'z' must be set explicitly in this function.
function test(s $s): t {
  return $s;
}
