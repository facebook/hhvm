<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function fun1(varray $x): varray { return $x; }
function fun2(darray $x): darray { return $x; }
function fun3(varray_or_darray $x): varray_or_darray { return $x; }
function fun4(array $x): array { return $x; }

function opt_fun1(?varray $x): ?varray { return $x; }
function opt_fun2(?darray $x): ?darray { return $x; }
function opt_fun3(?varray_or_darray $x): ?varray_or_darray { return $x; }
function opt_fun4(?array $x): ?array { return $x; }

function tuple_fun((bool, bool) $x): (bool, bool) { return $x; }
function shape_fun(shape('a' => bool, 'b' => bool) $x): shape('a' => bool, 'b' => bool) { return $x; }

function opt_tuple_fun(?(bool, bool) $x): ?(bool, bool) { return $x; }
function opt_shape_fun(?shape('a' => bool, 'b' => bool) $x): ?shape('a' => bool, 'b' => bool) { return $x; }

function inout_fun1(inout varray $x, $y) { $x = $y; }
function inout_fun2(inout darray $x, $y) { $x = $y; }
function inout_fun3(inout varray_or_darray $x, $y) { $x = $y; }
function inout_fun4(inout array $x, $y) { $x = $y; }

function opt_inout_fun1(inout ?varray $x, $y) { $x = $y; }
function opt_inout_fun2(inout ?darray $x, $y) { $x = $y; }
function opt_inout_fun3(inout ?varray_or_darray $x, $y) { $x = $y; }
function opt_inout_fun4(inout ?array $x, $y) { $x = $y; }

type T1 = varray;
type T2 = darray;
type T3 = varray_or_darray;
type T4 = array;
type T5 = (int, int);
type T6 = shape('x' => int, 'y' => int);
type T7 = T1;

function alias_fun1(T1 $x): T1 { return $x; }
function alias_fun2(T2 $x): T2 { return $x; }
function alias_fun3(T3 $x): T3 { return $x; }
function alias_fun4(T4 $x): T4 { return $x; }
function alias_fun5(T5 $x): T5 { return $x; }
function alias_fun6(T6 $x): T6 { return $x; }
function alias_fun7(T7 $x): T7 { return $x; }

function opt_alias_fun1(?T1 $x): ?T1 { return $x; }
function opt_alias_fun2(?T2 $x): ?T2 { return $x; }
function opt_alias_fun3(?T3 $x): ?T3 { return $x; }
function opt_alias_fun4(?T4 $x): ?T4 { return $x; }
function opt_alias_fun5(?T5 $x): ?T5 { return $x; }
function opt_alias_fun6(?T6 $x): ?T6 { return $x; }
function opt_alias_fun7(?T7 $x): ?T7 { return $x; }

class Pile {
  public static $logs = vec[];
}

function handler($errno, $errstr) {
  Pile::$logs[] = $errstr;
}

function test() {
  fun1(tuple(true, false));
  fun1(shape('a' => true, 'b' => false));
  fun1(varray[]);
  fun1(darray[]);
  fun1(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  fun2(tuple(true, false));
  fun2(shape('a' => true, 'b' => false));
  fun2(varray[]);
  fun2(darray[]);
  fun2(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  fun3(tuple(true, false));
  fun3(shape('a' => true, 'b' => false));
  fun3(varray[]);
  fun3(darray[]);
  fun3(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  fun4(tuple(true, false));
  fun4(shape('a' => true, 'b' => false));
  fun4(varray[]);
  fun4(darray[]);
  fun4(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  opt_fun1(tuple(true, false));
  opt_fun1(shape('a' => true, 'b' => false));
  opt_fun1(varray[]);
  opt_fun1(darray[]);
  opt_fun1(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_fun1(null);

  opt_fun2(tuple(true, false));
  opt_fun2(shape('a' => true, 'b' => false));
  opt_fun2(varray[]);
  opt_fun2(darray[]);
  opt_fun2(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_fun2(null);

  opt_fun3(tuple(true, false));
  opt_fun3(shape('a' => true, 'b' => false));
  opt_fun3(varray[]);
  opt_fun3(darray[]);
  opt_fun3(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_fun3(null);

  opt_fun4(tuple(true, false));
  opt_fun4(shape('a' => true, 'b' => false));
  opt_fun4(varray[]);
  opt_fun4(darray[]);
  opt_fun4(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_fun4(null);

  tuple_fun(tuple(true, false));
  tuple_fun(shape('a' => true, 'b' => false));
  tuple_fun(varray[]);
  tuple_fun(darray[]);
  tuple_fun(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  shape_fun(shape('a' => true, 'b' => false));
  shape_fun(tuple(true, false));
  shape_fun(varray[]);
  shape_fun(darray[]);
  shape_fun(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  opt_tuple_fun(tuple(true, false));
  opt_tuple_fun(shape('a' => true, 'b' => false));
  opt_tuple_fun(varray[]);
  opt_tuple_fun(darray[]);
  opt_tuple_fun(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_tuple_fun(null);

  opt_shape_fun(shape('a' => true, 'b' => false));
  opt_shape_fun(tuple(true, false));
  opt_shape_fun(varray[]);
  opt_shape_fun(darray[]);
  opt_shape_fun(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_shape_fun(null);

  alias_fun1(tuple(true, false));
  alias_fun1(shape('a' => true, 'b' => false));
  alias_fun1(varray[]);
  alias_fun1(darray[]);
  alias_fun1(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  alias_fun2(tuple(true, false));
  alias_fun2(shape('a' => true, 'b' => false));
  alias_fun2(varray[]);
  alias_fun2(darray[]);
  alias_fun2(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  alias_fun3(tuple(true, false));
  alias_fun3(shape('a' => true, 'b' => false));
  alias_fun3(varray[]);
  alias_fun3(darray[]);
  alias_fun3(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  alias_fun4(tuple(true, false));
  alias_fun4(shape('a' => true, 'b' => false));
  alias_fun4(varray[]);
  alias_fun4(darray[]);
  alias_fun4(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  alias_fun5(tuple(true, false));
  alias_fun5(shape('a' => true, 'b' => false));
  alias_fun5(varray[]);
  alias_fun5(darray[]);
  alias_fun5(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  alias_fun6(tuple(true, false));
  alias_fun6(shape('a' => true, 'b' => false));
  alias_fun6(varray[]);
  alias_fun6(darray[]);
  alias_fun6(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  alias_fun7(tuple(true, false));
  alias_fun7(shape('a' => true, 'b' => false));
  alias_fun7(varray[]);
  alias_fun7(darray[]);
  alias_fun7(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  opt_alias_fun1(tuple(true, false));
  opt_alias_fun1(shape('a' => true, 'b' => false));
  opt_alias_fun1(varray[]);
  opt_alias_fun1(darray[]);
  opt_alias_fun1(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_alias_fun1(null);

  opt_alias_fun2(tuple(true, false));
  opt_alias_fun2(shape('a' => true, 'b' => false));
  opt_alias_fun2(varray[]);
  opt_alias_fun2(darray[]);
  opt_alias_fun2(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_alias_fun2(null);

  opt_alias_fun3(tuple(true, false));
  opt_alias_fun3(shape('a' => true, 'b' => false));
  opt_alias_fun3(varray[]);
  opt_alias_fun3(darray[]);
  opt_alias_fun3(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_alias_fun3(null);

  opt_alias_fun4(tuple(true, false));
  opt_alias_fun4(shape('a' => true, 'b' => false));
  opt_alias_fun4(varray[]);
  opt_alias_fun4(darray[]);
  opt_alias_fun4(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_alias_fun4(null);

  opt_alias_fun5(tuple(true, false));
  opt_alias_fun5(shape('a' => true, 'b' => false));
  opt_alias_fun5(varray[]);
  opt_alias_fun5(darray[]);
  opt_alias_fun5(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_alias_fun5(null);

  opt_alias_fun6(tuple(true, false));
  opt_alias_fun6(shape('a' => true, 'b' => false));
  opt_alias_fun6(varray[]);
  opt_alias_fun6(darray[]);
  opt_alias_fun6(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_alias_fun6(null);

  opt_alias_fun7(tuple(true, false));
  opt_alias_fun7(shape('a' => true, 'b' => false));
  opt_alias_fun7(varray[]);
  opt_alias_fun7(darray[]);
  opt_alias_fun7(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  opt_alias_fun7(null);

  $x = varray[];
  inout_fun1(inout $x, tuple(true, false));
  $x = varray[];
  inout_fun1(inout $x, shape('a' => true, 'b' => false));
  $x = varray[];
  inout_fun1(inout $x, varray[]);
  $x = varray[];
  inout_fun1(inout $x, darray[]);
  $x = varray[];
  inout_fun1(inout $x, __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  $x = darray[];
  inout_fun2(inout $x, tuple(true, false));
  $x = darray[];
  inout_fun2(inout $x, shape('a' => true, 'b' => false));
  $x = darray[];
  inout_fun2(inout $x, varray[]);
  $x = darray[];
  inout_fun2(inout $x, darray[]);
  $x = darray[];
  inout_fun2(inout $x, __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  $x = darray[];
  inout_fun3(inout $x, tuple(true, false));
  $x = darray[];
  inout_fun3(inout $x, shape('a' => true, 'b' => false));
  $x = darray[];
  inout_fun3(inout $x, varray[]);
  $x = darray[];
  inout_fun3(inout $x, darray[]);
  $x = darray[];
  inout_fun3(inout $x, __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  inout_fun4(inout $x, tuple(true, false));
  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  inout_fun4(inout $x, shape('a' => true, 'b' => false));
  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  inout_fun4(inout $x, varray[]);
  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  inout_fun4(inout $x, darray[]);
  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  inout_fun4(inout $x, __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));

  $x = varray[];
  opt_inout_fun1(inout $x, tuple(true, false));
  $x = varray[];
  opt_inout_fun1(inout $x, shape('a' => true, 'b' => false));
  $x = varray[];
  opt_inout_fun1(inout $x, varray[]);
  $x = varray[];
  opt_inout_fun1(inout $x, darray[]);
  $x = varray[];
  opt_inout_fun1(inout $x, __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  $x = varray[];
  opt_inout_fun1(inout $x, null);

  $x = darray[];
  opt_inout_fun2(inout $x, tuple(true, false));
  $x = darray[];
  opt_inout_fun2(inout $x, shape('a' => true, 'b' => false));
  $x = darray[];
  opt_inout_fun2(inout $x, varray[]);
  $x = darray[];
  opt_inout_fun2(inout $x, darray[]);
  $x = darray[];
  opt_inout_fun2(inout $x, __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  $x = darray[];
  opt_inout_fun2(inout $x, null);

  $x = darray[];
  opt_inout_fun3(inout $x, tuple(true, false));
  $x = darray[];
  opt_inout_fun3(inout $x, shape('a' => true, 'b' => false));
  $x = darray[];
  opt_inout_fun3(inout $x, varray[]);
  $x = darray[];
  opt_inout_fun3(inout $x, darray[]);
  $x = darray[];
  opt_inout_fun3(inout $x, __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  $x = darray[];
  opt_inout_fun3(inout $x, null);

  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  opt_inout_fun4(inout $x, tuple(true, false));
  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  opt_inout_fun4(inout $x, shape('a' => true, 'b' => false));
  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  opt_inout_fun4(inout $x, varray[]);
  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  opt_inout_fun4(inout $x, darray[]);
  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  opt_inout_fun4(inout $x, __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  $x = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
  opt_inout_fun4(inout $x, null);
}


<<__EntryPoint>>
function main_type_hints() {
  set_error_handler(fun('handler'));

  test();

  $seen = keyset[];
  foreach (Pile::$logs as $log) {
    if (isset($seen[$log])) { continue; }
    $seen[] = $log;
    print($log."\n\n");
  }
}
