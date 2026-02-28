<?hh

function f(KeyedTraversable<int, string> $t): void {
  foreach ($t as $k @ $v) {
    expect_int($k);
    expect_string($v);
  }
}

function expect_int(int $x): void {}
function expect_string(int $x): void {}
