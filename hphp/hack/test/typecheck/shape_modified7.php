<?hh

type s = shape('x' => ?int);

function test(bool $b, s $s): int {

  if ($b) {
    $x = 5;
  } else {
    $x = 4;
  }

  if ($b) {
    $s['x'] = $x;
  }

  return $x;
}
