<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

final class NamedCaller {
  public function generic<T>(named T $value): T { return $value; }
}
function expect_int(int $_): void {}
function expect_string(string $_): void {}

function test(): void {
  $f = HH\meth_caller(NamedCaller::class, 'generic');
  expect_int($f(new NamedCaller(), value=3));
  expect_string($f(new NamedCaller(), value='ok'));
}
