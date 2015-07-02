<?hh // strict

/**
 * Argument is not a shape
 */
function test(int $s): void {
  Shapes::idx($s, 'x');
}
