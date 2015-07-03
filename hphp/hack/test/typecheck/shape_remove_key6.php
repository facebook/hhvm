<?hh

/**
 * Removing optional field allows to omit it even when fields are only
 * partially known.
 */
type s = shape('x' => int);

type t = shape(
  'x' => int,
  'z' => ?bool,
);

function test(s $s): t {
  Shapes::removeKey($s, 'z');
  return $s;
}
