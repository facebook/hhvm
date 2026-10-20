<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class Box<T> {}
class MethodGeneric {
  public function method_id<T>(named Box<T> $box, named T $x): T { return $x; }
}
function expect_string(string $_): void {}

function test(Box<int> $box): void {
  $f = HH\meth_caller(MethodGeneric::class, 'method_id');
  $f(new MethodGeneric(), x='wrong', box=$box); // Error
  $result = $f(new MethodGeneric(), box=$box, x=1);
  expect_string($result); // Error
}
