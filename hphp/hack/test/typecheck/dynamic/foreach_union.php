<?hh

class C {}

function expect_vec_int(vec<int> $x): void {}

function good_case(dynamic $dyn, vec<vec<int>> $vec): void {
  if ($dyn) {
    $vec = $dyn;
  }
  foreach ($vec as $x) {
    expect_vec_int($x); // error, result is dynamic | vec<int>
  }
}

function bad_case(dynamic $dyn, C $c): void {
  if ($dyn) {
    $c = $dyn;
  }
  foreach ($c /* error */ as $x) {
  }
}
