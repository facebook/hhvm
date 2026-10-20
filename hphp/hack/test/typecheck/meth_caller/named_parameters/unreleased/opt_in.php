<?hh
<<file: __EnableUnstableFeatures(
  'named_parameters',
  'named_parameters_meth_caller',
)>>

final class OptInNamedCaller {
  public function method(named int $x): int {
    return $x;
  }
}

function expect_int(int $_): void {}

function test_opt_in_named_caller(): void {
  $caller = HH\meth_caller(OptInNamedCaller::class, 'method');
  expect_int($caller(new OptInNamedCaller(), x=1));
}
//
