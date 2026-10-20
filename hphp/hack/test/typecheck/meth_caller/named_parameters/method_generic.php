<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class Box<T> {}
class MethodGeneric {
  public function method_id<T>(named Box<T> $box, named T $x): T { return $x; }
}
function expect_int(int $_): void {}
function expect_string(string $_): void {}

function test(Box<int> $ibox, Box<string> $sbox): void {
  $f = HH\meth_caller(MethodGeneric::class, 'method_id');
  expect_int($f(new MethodGeneric(), x=1, box=$ibox));
  expect_string($f(new MethodGeneric(), box=$sbox, x='ok'));
}
