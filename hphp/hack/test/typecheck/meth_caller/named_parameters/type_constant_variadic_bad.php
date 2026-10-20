<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

abstract class TypeConstant {
  abstract const type Item as arraykey;
  public function variadic(named this::Item...): void {}
}
class IntItem extends TypeConstant { const type Item = int; }
class StringItem extends TypeConstant { const type Item = string; }

function test(IntItem $ints, StringItem $strings): void {
  $f = HH\meth_caller(TypeConstant::class, 'variadic');
  $f($ints, x='wrong'); // Error
  $f($strings, x=1); // Error
}
