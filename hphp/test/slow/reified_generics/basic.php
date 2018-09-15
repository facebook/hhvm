<?hh

function g<reified T1, reified T2>() {
  var_dump(__hhvm_intrinsics\get_reified_type(T1));
  var_dump(__hhvm_intrinsics\get_reified_type(T2));
}

function f<reified T1, reified T2>($x, $y) {
  g<reified (int, (T1, T2)), reified T1>();
}

f<reified (int, int), reified int>(1, 2);
