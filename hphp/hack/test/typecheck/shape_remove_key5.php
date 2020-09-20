<?hh //strict

/**
 * removeKey allows unsetting any key, but it still must be statically
 * known
 */
type s = shape(
  'x' => int,
  'y' => string,
);

function test(s $s, string $k): void {
  Shapes::removeKey(inout $s, $k);
}
