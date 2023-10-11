<?hh

function testBools(dynamic $x): void {
  $y = $x && true; // $y : bool
  expect_boolean($y);
  $y = $x || false; // $y : bool
  expect_boolean($y);
  $y = !$x; // $y : bool
  expect_boolean($y);
  $y = $x === 5; // $y : bool
  expect_boolean($y);
  $y = $x == 5; // $y : bool
  expect_boolean($y);
  if ($x) // valid, no sketchy null check warning
  {
  }
}

function expect_boolean(bool $_):void {}
