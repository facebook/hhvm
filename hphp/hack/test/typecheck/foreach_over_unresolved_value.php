<?hh

function expect_arraykey(arraykey $_): void {}

function test(bool $b, vec<string> $x, vec<int> $y): void {
  if ($b) {
    $z = $x;
  } else {
    $z = $y;
  }
  foreach ($z as $v) {
    expect_arraykey($v);
  }
}
