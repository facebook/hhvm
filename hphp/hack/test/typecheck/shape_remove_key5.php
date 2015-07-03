<?hh //strict

/**
 * removeKey allows to unset any key, but it still must be statically
 * known
 */
type s = shape(
  'x' => int,
  'y' => string,
);

function test(s $s, string $k): void {
  Shapes::removeKey($s, $k);
}
