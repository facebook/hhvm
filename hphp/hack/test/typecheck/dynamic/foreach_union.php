<?hh

class C {}

function expect_int(int $x): void {}

function good_case(dynamic $dyn, vec<int> $vec): void {
  if ($dyn) {
    $vec = $dyn;
  }
  foreach ($vec as $x) {
    expect_int($x); // error, result is dynamic | int
  }
}

function bad_case(dynamic $dyn, C $c): void {
  if ($dyn) {
    $c = $dyn;
  }
  foreach ($c /* error */ as $x) {
  }
}
