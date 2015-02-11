<?hh

function test(): int {
  $w = 0;
  $x = 1;
  $y = 2;
  $z = 3;
  for ($i = 0; $i < 3; $i++) {
    $w = $y;
    for ($j = 0; $j < 3; $j++) {
      $x = $w;
      $y = $z;
      $z = 'hello';
    }
  }
  return $x;
}
