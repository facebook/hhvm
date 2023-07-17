<?hh

<<file:__EnableUnstableFeatures('case_types')>>

class A {}
class B {}

case type C1 = int | string | float | null;
case type C2 = bool | A;
case type C3 = B | C2;

<<__EntryPoint>>
function main() :mixed{
  $x = __hhvm_intrinsics\launder_value(1);
  var_dump($x is C1);
  var_dump($x is C2);
  var_dump($x is C3);
}
