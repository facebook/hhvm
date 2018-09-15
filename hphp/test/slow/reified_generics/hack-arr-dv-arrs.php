<?hh

function f<reified T>() {
  var_dump(__hhvm_intrinsics\get_reified_type(T));
}

f<reified int>();
