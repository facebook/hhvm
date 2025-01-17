<?hh

<<file:__EnableUnstableFeatures('case_types')>>

case type C = int | T;
type T = (C, C);

<<__EntryPoint>>
function main() {
  var_dump(HH\ReifiedGenerics\get_type_structure<C>());
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  var_dump(HH\ReifiedGenerics\get_type_structure<(C, C)>());
  var_dump(__hhvm_intrinsics\isTypeStructShallow<C>(3));

}

