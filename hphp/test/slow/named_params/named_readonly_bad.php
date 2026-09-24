<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class Target {
  public function __construct(named readonly int $r, named int $m) {
    echo "unexpected constructor call\n";
  }
}

function target(named readonly int $r, named int $m): void {
  echo "unexpected function call\n";
}

function expect_rejection($f): void {
  try {
    $f();
    echo "missing exception\n";
  } catch (ReadonlyViolationException $e) {
    echo "rejected\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $ro = readonly 1;
  expect_rejection(() ==> new Target(r = 2, m = $ro));
  expect_rejection(() ==> new Target(m = $ro, r = 2));
  expect_rejection(() ==> target(r = 2, m = $ro));
  expect_rejection(() ==> target(m = $ro, r = 2));
  expect_rejection(() ==> target(m = readonly 1, r = 2));
  $f = __hhvm_intrinsics\launder_value(target<>);
  expect_rejection(() ==> $f(m = $ro, r = 2));
}
