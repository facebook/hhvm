<?hh

function expect_arraykey(arraykey $_): void {}

function test(bool $b, dict<string, string> $x, dict<int, string> $y): void {
  if ($b) {
    $z = $x;
  } else {
    $z = $y;
  }
  foreach ($z as $k => $_) {
    expect_arraykey($k);
  }
}
