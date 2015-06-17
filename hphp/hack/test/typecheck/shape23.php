<?hh

/**
 * Structural subtyping of ad-hoc shapes
 */

type t = shape('x' => int);

function test(s $s): t {
  return shape('x' => 4, 'y' => 'aaa');
}
