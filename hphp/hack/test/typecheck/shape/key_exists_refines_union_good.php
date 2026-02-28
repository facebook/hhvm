<?hh

function expect_arraykey(arraykey $_): void {}

function test1(
  bool $b,
  shape(?'x' => int) $s1,
  shape(?'x' => string) $s2,
): void {
  if ($b) {
    $s = $s1;
  } else {
    $s = $s2;
  }
  if (Shapes::keyExists($s, 'x')) {
    expect_arraykey($s['x']);
  }
}

function test2(
  bool $b,
  shape(?'x' => int, ...) $s1,
  shape(?'x' => string) $s2,
): void {
  if ($b) {
    $s = $s1;
  } else {
    $s = $s2;
  }
  if (Shapes::keyExists($s, 'x')) {
    expect_arraykey($s['x']);
  }
}

function test3(
  bool $b,
  shape(?'x' => int, ...) $s1,
  shape(?'x' => string, ...) $s2,
): void {
  if ($b) {
    $s = $s1;
  } else {
    $s = $s2;
  }
  if (Shapes::keyExists($s, 'x')) {
    expect_arraykey($s['x']);
  }
}
