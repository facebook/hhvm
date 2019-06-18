<?hh

<<__EntryPoint>>
function main() {
  $x = vec[1, 2, 3];
  var_dump(serialize(__hhvm_intrinsics\launder_value($x)));
  var_dump(serialize(__hhvm_intrinsics\launder_value($x)));
}
