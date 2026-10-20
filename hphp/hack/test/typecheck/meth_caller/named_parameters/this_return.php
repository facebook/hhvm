<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class ThisReturn {
  public function tag(named string $x): this { return $this; }
}
class Child extends ThisReturn {}
function expect_child(Child $_): void {}

function test(Child $child): void {
  $f = HH\meth_caller(ThisReturn::class, 'tag');
  expect_child($f($child, x='ok'));
}
