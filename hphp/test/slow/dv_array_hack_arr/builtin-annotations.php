<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function return_values() :mixed{
  $x = __hhvm_intrinsics\dummy_varray_builtin(vec[1,2,3]);
  var_dump(HH\is_php_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);
  echo "====================================================\n";

  $x = __hhvm_intrinsics\dummy_darray_builtin(dict['a'=>1,'b'=>2,'c'=>3]);
  var_dump(HH\is_php_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);
  echo "====================================================\n";

  $x = __hhvm_intrinsics\dummy_varr_or_darr_builtin(vec[1,2,3]);
  var_dump(HH\is_php_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);
  echo "====================================================\n";

  $x =
    __hhvm_intrinsics\dummy_varr_or_darr_builtin(dict['a'=>1,'b'=>2,'c'=>3]);
  var_dump(HH\is_php_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);
  echo "====================================================\n";
}

function parameters() :mixed{
  __hhvm_intrinsics\dummy_varray_builtin(vec[1,2,3]);
  try { __hhvm_intrinsics\dummy_varray_builtin(dict['a'=>1,'b'=>2,'c'=>3]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  echo "====================================================\n";

  try { __hhvm_intrinsics\dummy_darray_builtin(vec[1,2,3]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  __hhvm_intrinsics\dummy_darray_builtin(dict['a'=>1,'b'=>2,'c'=>3]);
  echo "====================================================\n";

  __hhvm_intrinsics\dummy_varr_or_darr_builtin(vec[1,2,3]);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(dict['a'=>1,'b'=>2,'c'=>3]);
  echo "====================================================\n";
}


<<__EntryPoint>>
function main_builtin_annotations(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });
  return_values();
  parameters();
}
