<?hh // strict

type s = shape('x' => int);

/**
 * Default has the wrong type
 */
function test(s $s, string $default): void {
  Shapes::idx($s, 'x', $default);
}
