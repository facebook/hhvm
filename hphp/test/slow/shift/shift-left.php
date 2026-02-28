<?hh

<<__EntryPoint>>
function main(): void {
  var_dump(__hhvm_intrinsics\launder_value(1) << 2 << 3);
  var_dump(__hhvm_intrinsics\launder_value(1) << __hhvm_intrinsics\launder_value(2) << __hhvm_intrinsics\launder_value(3));
  var_dump(__hhvm_intrinsics\launder_value(1) << 5);

  var_dump(__hhvm_intrinsics\launder_value(1) << 20 << 11);
  var_dump(__hhvm_intrinsics\launder_value(1) << 31);

  var_dump(__hhvm_intrinsics\launder_value(1) << 20 << 12);
  var_dump(__hhvm_intrinsics\launder_value(1) << 32);

  var_dump(__hhvm_intrinsics\launder_value(1) << 20 << 60);
  var_dump(__hhvm_intrinsics\launder_value(1) << 80);

  var_dump(__hhvm_intrinsics\launder_value(1) << 62 << 62);
  var_dump(__hhvm_intrinsics\launder_value(1) << 124);
}
