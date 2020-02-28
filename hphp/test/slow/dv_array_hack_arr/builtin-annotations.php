<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function return_values() {
  $x = __hhvm_intrinsics\dummy_varray_builtin(varray[1,2,3]);
  var_dump(is_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);
  echo "====================================================\n";

  $x = __hhvm_intrinsics\dummy_darray_builtin(darray['a'=>1,'b'=>2,'c'=>3]);
  var_dump(is_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);
  echo "====================================================\n";

  $x = __hhvm_intrinsics\dummy_varr_or_darr_builtin(varray[1,2,3]);
  var_dump(is_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);
  echo "====================================================\n";

  $x =
    __hhvm_intrinsics\dummy_varr_or_darr_builtin(darray['a'=>1,'b'=>2,'c'=>3]);
  var_dump(is_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);
  echo "====================================================\n";

  $x = __hhvm_intrinsics\dummy_array_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1,2,3]));
  var_dump(is_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);
  echo "====================================================\n";
}

function parameters() {
  __hhvm_intrinsics\dummy_varray_builtin(varray[1,2,3]);
  try { __hhvm_intrinsics\dummy_varray_builtin(darray['a'=>1,'b'=>2,'c'=>3]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { __hhvm_intrinsics\dummy_varray_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1,2,3])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  echo "====================================================\n";

  try { __hhvm_intrinsics\dummy_darray_builtin(varray[1,2,3]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  __hhvm_intrinsics\dummy_darray_builtin(darray['a'=>1,'b'=>2,'c'=>3]);
  try { __hhvm_intrinsics\dummy_darray_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1,2,3])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  echo "====================================================\n";

  __hhvm_intrinsics\dummy_varr_or_darr_builtin(varray[1,2,3]);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(darray['a'=>1,'b'=>2,'c'=>3]);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1,2,3]));
  echo "====================================================\n";

  try { __hhvm_intrinsics\dummy_array_builtin(varray[1,2,3]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { __hhvm_intrinsics\dummy_array_builtin(darray['a'=>1,'b'=>2,'c'=>3]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  __hhvm_intrinsics\dummy_array_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1,2,3]));
  echo "====================================================\n";
}


<<__EntryPoint>>
function main_builtin_annotations() {
return_values();
parameters();
}
