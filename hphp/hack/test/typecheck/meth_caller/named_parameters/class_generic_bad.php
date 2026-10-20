<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class ClassGeneric<T> {
  public function class_id(named T $x): T { return $x; }
}
function expect_string(string $_): void {}

function test(): void {
  $f = HH\meth_caller(ClassGeneric::class, 'class_id');
  $f(new ClassGeneric<int>(), x='wrong'); // Error
  $f(new ClassGeneric<string>(), x=1); // Error
  $result = $f(new ClassGeneric<int>(), x=1);
  expect_string($result); // Error
}
