<?hh

function f(int $x): int {
  $x -= 4;
  $x += 4;
  $x *= 4;

  $arr = vec[1, 2, 3, 4]; $i = 2;
  $arr[$i - 1] += 4;

  return $x;
}
