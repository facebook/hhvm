<?hh

type s = shape('x' => int);

/**
 * Too few arguments
 */
function test(s $s): void {
  Shapes::idx($s);
}
