<?hh // partial

/**
 * Removing optional field allows omitting it even when fields are only
 * partially known.
 */
type s = shape('x' => int);

type t = shape(
  'x' => int,
  ?'z' => bool,
);

function test(s $s): t {
  Shapes::removeKey(inout $s, 'z');
  return $s;
}
