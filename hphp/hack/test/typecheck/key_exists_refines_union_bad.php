<?hh // strict

function expect_int(int $_): void {}

function test(bool $b, shape(?'x' => int) $s1, shape(...) $s2): void {
  if ($b) {
    $s = $s1;
  } else {
    $s = $s2;
  }
  if (Shapes::keyExists($s, 'x')) {
    expect_int($s['x']);
  }
}
