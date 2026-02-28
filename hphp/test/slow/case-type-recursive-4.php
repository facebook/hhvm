<?hh

<<file:__EnableUnstableFeatures('case_types')>>

case type C = int | T;
type T = (C, C);

<<__EntryPoint>>
function main() {
  $x = __hhvm_intrinsics\launder_value(12);
  var_dump($x is C);
}

