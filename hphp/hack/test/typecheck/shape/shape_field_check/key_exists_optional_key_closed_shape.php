<?hh

type s = shape(?'x' => int);

function test(s $s): bool {
  return Shapes::keyExists($s, 'x');
}
