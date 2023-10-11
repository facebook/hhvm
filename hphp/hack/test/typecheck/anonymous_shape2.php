<?hh

/**
 * Anonymous shape return type
 */

function f(): shape('x' => int) {
  return shape('x' => 4);
}

function test(
): shape(
  'y' => int,
  ...
) {
  return f();
}
