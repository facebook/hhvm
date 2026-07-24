<?hh

function test_mixed<<<__Enforceable>> reify T>(mixed $x): void {
  if ($x is T) {
    hh_show($x);
  } else {
    hh_show($x);
  }
}

function test_identity<<<__Enforceable>> reify T>(T $x): void {
  if ($x is T) {
    hh_expect_equivalent<T>($x);
  } else {
    hh_expect_equivalent<nothing>($x);
  }
}

function test_bounded<<<__Enforceable>> reify T as arraykey>(float $x): void {
  if ($x is T) {
    hh_expect_equivalent<nothing>($x);
  } else {
    hh_expect_equivalent<float>($x);
  }
}
