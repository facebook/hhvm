<?hh

function g<reify T1, reify T2>() {
  var_dump(__hhvm_intrinsics\get_reified_type(T1));
  var_dump(__hhvm_intrinsics\get_reified_type(T2));
}

function f<reify T1, reify T2>($x, $y) {
  g<reify (int, (T1, T2)), reify T1>();
}

f<reify (int, int), reify int>(1, 2);
