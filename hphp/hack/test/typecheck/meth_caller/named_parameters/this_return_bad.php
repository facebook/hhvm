<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class ThisReturn {
  public function tag(named string $x): this { return $this; }
}
class Child extends ThisReturn {}
function expect_child(Child $_): void {}

function test(Child $child): void {
  $f = HH\meth_caller(ThisReturn::class, 'tag');
  $f($child, x=1); // Error
  $base = $f(new ThisReturn(), x='ok');
  expect_child($base); // Error
}
