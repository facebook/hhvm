<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function return_values(): void {
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

  echo "= KindOfDArray  ====================================================\n";
  $x =__hhvm_intrinsics\dummy_kindofdarray_builtin();
  var_dump(is_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);

  echo "= KindOfVArray  ====================================================\n";
  $x = __hhvm_intrinsics\dummy_kindofvarray_builtin();
  var_dump(is_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump($x);
  echo "====================================================\n";

  echo "= KindOfVArray KindOfDArray  ====================================================\n";
  $d = __hhvm_intrinsics\dummy_kindofdarray_builtin();
  $v = __hhvm_intrinsics\dummy_kindofvarray_builtin();
  var_dump($d === $v);
  var_dump($d == $v);
  $d[0] = 0;
  $v[] = 0;
  var_dump($d === $v);
  var_dump($d == $v);

  echo "= awaitable ====================================================\n";
  list($ma, $da, $di) = HH\Asio\join(dummy_await());
  var_dump(is_array($ma));
  var_dump(is_darray($da));
  var_dump(is_dict($di));
}

async function dummy_await(): Awaitable<(array, darray, dict)> {
  return tuple(
    await __hhvm_intrinsics\dummy_array_await(),
    await __hhvm_intrinsics\dummy_darray_await(),
    await __hhvm_intrinsics\dummy_dict_await(),
  );
}

function parameters(): void {
  __hhvm_intrinsics\dummy_varray_builtin(varray[1,2,3]);
  __hhvm_intrinsics\dummy_varray_builtin(darray['a'=>1,'b'=>2,'c'=>3]);
  __hhvm_intrinsics\dummy_varray_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1,2,3]));
  __hhvm_intrinsics\dummy_varray_builtin(__hhvm_intrinsics\dummy_kindofvarray_builtin());
  echo "====================================================\n";

  __hhvm_intrinsics\dummy_darray_builtin(varray[1,2,3]);
  __hhvm_intrinsics\dummy_darray_builtin(darray['a'=>1,'b'=>2,'c'=>3]);
  __hhvm_intrinsics\dummy_darray_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1,2,3]));
  __hhvm_intrinsics\dummy_darray_builtin(__hhvm_intrinsics\dummy_kindofdarray_builtin());
  echo "====================================================\n";

  __hhvm_intrinsics\dummy_varr_or_darr_builtin(varray[1,2,3]);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(darray['a'=>1,'b'=>2,'c'=>3]);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1,2,3]));
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(__hhvm_intrinsics\dummy_kindofdarray_builtin());
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(__hhvm_intrinsics\dummy_kindofvarray_builtin());
  echo "====================================================\n";

  __hhvm_intrinsics\dummy_array_builtin(varray[1,2,3]);
  __hhvm_intrinsics\dummy_array_builtin(darray['a'=>1,'b'=>2,'c'=>3]);
  __hhvm_intrinsics\dummy_array_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1,2,3]));
  __hhvm_intrinsics\dummy_array_builtin(__hhvm_intrinsics\dummy_kindofdarray_builtin());
  __hhvm_intrinsics\dummy_array_builtin(__hhvm_intrinsics\dummy_kindofvarray_builtin());
  echo "====================================================\n";

}

function cast(): void {
  echo "= cast ===================================================\n";
  var_dump(
    __hhvm_intrinsics\dummy_cast_to_kindofarray(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1,2,3]))
      |> is_darray($$) || is_varray($$),
  );
  $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray[1,2,3]);
  var_dump($a);
  var_dump(is_darray($a));
  var_dump(is_varray($a));
  $d = __hhvm_intrinsics\dummy_cast_to_kindofdarray(varray[1,2,3]);
  var_dump($d);
  var_dump(is_darray($d));
  var_dump(is_varray($d));
  $v = __hhvm_intrinsics\dummy_cast_to_kindofvarray(darray['a' => 1, 'b' => 2]);
  var_dump($v);
  var_dump(is_darray($v));
  var_dump(is_varray($v));
}

<<__EntryPoint>>
function main_builtin_annotations() {
  return_values();
  parameters();
  cast();
}
