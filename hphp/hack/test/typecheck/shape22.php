<?hh

/**
 * Structural subtyping of declared shapes
 */

type s = shape(
  'x' => int,
  'y' => string,
);

type t = shape('x' => num);

function test(s $s): t {
  return $s;
}
