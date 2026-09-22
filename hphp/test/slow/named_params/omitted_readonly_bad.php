<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function positional(int $r, named readonly int $z = 2): void {
  echo "unexpected positional call\n";
}

class NamedArgs {
  public function __construct(
    int $p = 0,
    named readonly int $a = 2,
    named int $b = 3,
  ) {
    echo "unexpected named call\n";
  }
}

<<__NEVER_INLINE>>
function indirect($f): void {
  $f(readonly 1);
}

function expect_exception($f): void {
  try {
    $f();
    echo "missing exception\n";
  } catch (Exception $e) {
    echo get_class($e).': '.$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  expect_exception(() ==> positional(readonly 1));
  expect_exception(() ==> indirect(
    __hhvm_intrinsics\launder_value(positional<>),
  ));
  expect_exception(() ==> new NamedArgs(b=readonly 3));
  $class = __hhvm_intrinsics\launder_value(NamedArgs::class);
  expect_exception(() ==> new $class(b=readonly 3));
  expect_exception(() ==> new NamedArgs(p=readonly 1));
  expect_exception(() ==> new NamedArgs(missing=readonly 1));
}
