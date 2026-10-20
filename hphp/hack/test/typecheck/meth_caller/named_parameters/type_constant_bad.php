<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

abstract class TypeConstant {
  abstract const type Item as arraykey;
  public function const_id(named this::Item $x): this::Item { return $x; }
}
class IntItem extends TypeConstant { const type Item = int; }
class StringItem extends TypeConstant { const type Item = string; }
function expect_string(string $_): void {}

function test(IntItem $ints, StringItem $strings): void {
  $f = HH\meth_caller(TypeConstant::class, 'const_id');
  $f($ints, x='wrong'); // Error
  $f($strings, x=1); // Error
  $result = $f($ints, x=1);
  expect_string($result); // Error
}
