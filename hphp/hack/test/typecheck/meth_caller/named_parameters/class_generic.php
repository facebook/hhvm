<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class ClassGeneric<T> {
  public function class_id(named T $x): T { return $x; }
}
function expect_int(int $_): void {}
function expect_string(string $_): void {}

function test(): void {
  $f = HH\meth_caller(ClassGeneric::class, 'class_id');
  expect_int($f(new ClassGeneric<int>(), x=1));
  expect_string($f(new ClassGeneric<string>(), x='ok'));
}
