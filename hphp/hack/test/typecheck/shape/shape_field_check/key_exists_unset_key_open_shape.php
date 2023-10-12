<?hh // strict

type s = shape('x' => int, ...);

function test(s $s): bool {
  Shapes::removeKey(inout $s, 'x');
  return Shapes::keyExists($s, 'x');
}
