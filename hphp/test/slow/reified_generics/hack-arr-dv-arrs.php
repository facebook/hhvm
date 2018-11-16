<?hh

function f<reify T>() {
  var_dump(__hhvm_intrinsics\get_reified_type(T));
}

f<reify int>();
