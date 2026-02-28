<?hh

<<__EntryPoint>>
function main(): void {
  var_dump(__hhvm_intrinsics\launder_value(1 << 6) >> 2 >> 3);
  var_dump(__hhvm_intrinsics\launder_value(1 << 6) >> __hhvm_intrinsics\launder_value(2) >> __hhvm_intrinsics\launder_value(3));
  var_dump(__hhvm_intrinsics\launder_value(1 << 6) >> 5);

  var_dump(__hhvm_intrinsics\launder_value(1 << 34) >> 20 >> 11);
  var_dump(__hhvm_intrinsics\launder_value(1 << 34) >> 31);

  var_dump(__hhvm_intrinsics\launder_value(1 << 34) >> 20 >> 12);
  var_dump(__hhvm_intrinsics\launder_value(1 << 34) >> 32);

  var_dump(__hhvm_intrinsics\launder_value(1 << 63) >> 20 >> 60);
  var_dump(__hhvm_intrinsics\launder_value(1 << 63) >> 80);

  var_dump(__hhvm_intrinsics\launder_value(1 << 63) >> 62 >> 62);
  var_dump(__hhvm_intrinsics\launder_value(1 << 63) >> 124);
}
