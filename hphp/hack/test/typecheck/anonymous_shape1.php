<?hh

/**
 * Nested anonymous shape
 */
type s = shape('x' => shape('y' => int));

function test(): s {
  return shape('x' => shape('y' => 'aaa'));
}
