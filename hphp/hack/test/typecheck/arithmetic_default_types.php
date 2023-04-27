<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function testInc(bool $b): void {
  $v = Vector<_>{};
  // Relies on ++ defaulting to int
  if ($b) {
    $v[0]++;
  } else {
    $v[0] = 1;
  }
}

function testNeg(bool $b): void {
  $v = Vector<_>{};
  // Relies on - defaulting to int
  if ($b) {
    $v[0] = -$v[0];
  } else {
    $v[0] = 1;
  }
}
function testPlus(bool $b): void {
  $v = Vector<_>{};
  // Relies on += defaulting to int
  if ($b) {
    $v[0] += 2;
  } else {
    $v[0] = 1;
  }
}
