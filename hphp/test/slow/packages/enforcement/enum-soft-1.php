<?hh

module a.b;

function dyn_get($x) {
  var_dump($x::FOO);
}

<<__EntryPoint>>
function main_enum_soft_1() {
  var_dump(SoftEnumFoo::FOO);
  var_dump(SoftEnumClsFoo::FOO);

  echo "\nDynamic>>\n";
  dyn_get(__hhvm_intrinsics\launder_value("SoftEnumFoo"));
  dyn_get(__hhvm_intrinsics\launder_value("SoftEnumClsFoo"));
}
