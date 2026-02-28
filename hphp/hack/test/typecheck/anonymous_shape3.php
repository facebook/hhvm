<?hh

/**
 * Anonymous shape parameter type hint
 */

function test(shape('x' => int) $s): void {
  $s['y'];
}
