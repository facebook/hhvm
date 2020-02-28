<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function handler($errno, $errstr) { throw new Exception($errstr); }

function test1() {
  __hhvm_intrinsics\dummy_varray_builtin(tuple(true, false));
  __hhvm_intrinsics\dummy_varray_builtin(shape('a' => true, 'b' => false));
  __hhvm_intrinsics\dummy_varray_builtin(varray[]);
  __hhvm_intrinsics\dummy_varray_builtin(darray[]);
  __hhvm_intrinsics\dummy_varray_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  __hhvm_intrinsics\dummy_darray_builtin(tuple(true, false));
  __hhvm_intrinsics\dummy_darray_builtin(shape('a' => true, 'b' => false));
  __hhvm_intrinsics\dummy_darray_builtin(varray[]);
  __hhvm_intrinsics\dummy_darray_builtin(darray[]);
  __hhvm_intrinsics\dummy_darray_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  __hhvm_intrinsics\dummy_varr_or_darr_builtin(tuple(true, false));
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(shape('a' => true, 'b' => false));
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(varray[]);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(darray[]);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  __hhvm_intrinsics\dummy_array_builtin(tuple(true, false));
  __hhvm_intrinsics\dummy_array_builtin(shape('a' => true, 'b' => false));
  __hhvm_intrinsics\dummy_array_builtin(varray[]);
  __hhvm_intrinsics\dummy_array_builtin(darray[]);
  __hhvm_intrinsics\dummy_array_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
}

function test2($a, $b, $c, $d, $e) {
  $a = __hhvm_intrinsics\launder_value($a);
  $b = __hhvm_intrinsics\launder_value($b);
  $c = __hhvm_intrinsics\launder_value($c);
  $d = __hhvm_intrinsics\launder_value($d);
  $e = __hhvm_intrinsics\launder_value($e);

  __hhvm_intrinsics\dummy_varray_builtin($a);
  __hhvm_intrinsics\dummy_varray_builtin($b);
  __hhvm_intrinsics\dummy_varray_builtin($c);
  __hhvm_intrinsics\dummy_varray_builtin($d);
  __hhvm_intrinsics\dummy_varray_builtin($e);

  __hhvm_intrinsics\dummy_darray_builtin($a);
  __hhvm_intrinsics\dummy_darray_builtin($b);
  __hhvm_intrinsics\dummy_darray_builtin($c);
  __hhvm_intrinsics\dummy_darray_builtin($d);
  __hhvm_intrinsics\dummy_darray_builtin($e);

  __hhvm_intrinsics\dummy_varr_or_darr_builtin($a);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin($b);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin($c);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin($d);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin($e);

  __hhvm_intrinsics\dummy_array_builtin($a);
  __hhvm_intrinsics\dummy_array_builtin($b);
  __hhvm_intrinsics\dummy_array_builtin($c);
  __hhvm_intrinsics\dummy_array_builtin($d);
  __hhvm_intrinsics\dummy_array_builtin($e);

  set_error_handler(fun('handler'));

  try {
    __hhvm_intrinsics\dummy_varray_builtin($b);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
}


<<__EntryPoint>>
function main_type_hints_builtins() {
test1();
test2(
  tuple(true, false),
  shape('a' => true, 'b' => false),
  varray[],
  darray[],
  __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])
);
}
