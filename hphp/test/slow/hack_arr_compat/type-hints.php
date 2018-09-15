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

function throws1(varray $x) { return $x; }
function throws2($x): varray { return $x; }
function throws3(inout varray $x) { $x = darray[]; }

function handler($errno, $errstr) { throw new Exception($errstr); }

function test() {
  fun1(tuple(true, false));
  fun1(shape('a' => true, 'b' => false));
  fun1(varray[]);
  fun1(darray[]);
  fun1([]);

  fun2(tuple(true, false));
  fun2(shape('a' => true, 'b' => false));
  fun2(varray[]);
  fun2(darray[]);
  fun2([]);

  fun3(tuple(true, false));
  fun3(shape('a' => true, 'b' => false));
  fun3(varray[]);
  fun3(darray[]);
  fun3([]);

  fun4(tuple(true, false));
  fun4(shape('a' => true, 'b' => false));
  fun4(varray[]);
  fun4(darray[]);
  fun4([]);

  opt_fun1(tuple(true, false));
  opt_fun1(shape('a' => true, 'b' => false));
  opt_fun1(varray[]);
  opt_fun1(darray[]);
  opt_fun1([]);
  opt_fun1(null);

  opt_fun2(tuple(true, false));
  opt_fun2(shape('a' => true, 'b' => false));
  opt_fun2(varray[]);
  opt_fun2(darray[]);
  opt_fun2([]);
  opt_fun2(null);

  opt_fun3(tuple(true, false));
  opt_fun3(shape('a' => true, 'b' => false));
  opt_fun3(varray[]);
  opt_fun3(darray[]);
  opt_fun3([]);
  opt_fun3(null);

  opt_fun4(tuple(true, false));
  opt_fun4(shape('a' => true, 'b' => false));
  opt_fun4(varray[]);
  opt_fun4(darray[]);
  opt_fun4([]);
  opt_fun4(null);

  tuple_fun(tuple(true, false));
  tuple_fun(shape('a' => true, 'b' => false));
  tuple_fun(varray[]);
  tuple_fun(darray[]);
  tuple_fun([]);

  shape_fun(shape('a' => true, 'b' => false));
  shape_fun(tuple(true, false));
  shape_fun(varray[]);
  shape_fun(darray[]);
  shape_fun([]);

  opt_tuple_fun(tuple(true, false));
  opt_tuple_fun(shape('a' => true, 'b' => false));
  opt_tuple_fun(varray[]);
  opt_tuple_fun(darray[]);
  opt_tuple_fun([]);
  opt_tuple_fun(null);

  opt_shape_fun(shape('a' => true, 'b' => false));
  opt_shape_fun(tuple(true, false));
  opt_shape_fun(varray[]);
  opt_shape_fun(darray[]);
  opt_shape_fun([]);
  opt_shape_fun(null);

  alias_fun1(tuple(true, false));
  alias_fun1(shape('a' => true, 'b' => false));
  alias_fun1(varray[]);
  alias_fun1(darray[]);
  alias_fun1([]);

  alias_fun2(tuple(true, false));
  alias_fun2(shape('a' => true, 'b' => false));
  alias_fun2(varray[]);
  alias_fun2(darray[]);
  alias_fun2([]);

  alias_fun3(tuple(true, false));
  alias_fun3(shape('a' => true, 'b' => false));
  alias_fun3(varray[]);
  alias_fun3(darray[]);
  alias_fun3([]);

  alias_fun4(tuple(true, false));
  alias_fun4(shape('a' => true, 'b' => false));
  alias_fun4(varray[]);
  alias_fun4(darray[]);
  alias_fun4([]);

  alias_fun5(tuple(true, false));
  alias_fun5(shape('a' => true, 'b' => false));
  alias_fun5(varray[]);
  alias_fun5(darray[]);
  alias_fun5([]);

  alias_fun6(tuple(true, false));
  alias_fun6(shape('a' => true, 'b' => false));
  alias_fun6(varray[]);
  alias_fun6(darray[]);
  alias_fun6([]);

  alias_fun7(tuple(true, false));
  alias_fun7(shape('a' => true, 'b' => false));
  alias_fun7(varray[]);
  alias_fun7(darray[]);
  alias_fun7([]);

  opt_alias_fun1(tuple(true, false));
  opt_alias_fun1(shape('a' => true, 'b' => false));
  opt_alias_fun1(varray[]);
  opt_alias_fun1(darray[]);
  opt_alias_fun1([]);
  opt_alias_fun1(null);

  opt_alias_fun2(tuple(true, false));
  opt_alias_fun2(shape('a' => true, 'b' => false));
  opt_alias_fun2(varray[]);
  opt_alias_fun2(darray[]);
  opt_alias_fun2([]);
  opt_alias_fun2(null);

  opt_alias_fun3(tuple(true, false));
  opt_alias_fun3(shape('a' => true, 'b' => false));
  opt_alias_fun3(varray[]);
  opt_alias_fun3(darray[]);
  opt_alias_fun3([]);
  opt_alias_fun3(null);

  opt_alias_fun4(tuple(true, false));
  opt_alias_fun4(shape('a' => true, 'b' => false));
  opt_alias_fun4(varray[]);
  opt_alias_fun4(darray[]);
  opt_alias_fun4([]);
  opt_alias_fun4(null);

  opt_alias_fun5(tuple(true, false));
  opt_alias_fun5(shape('a' => true, 'b' => false));
  opt_alias_fun5(varray[]);
  opt_alias_fun5(darray[]);
  opt_alias_fun5([]);
  opt_alias_fun5(null);

  opt_alias_fun6(tuple(true, false));
  opt_alias_fun6(shape('a' => true, 'b' => false));
  opt_alias_fun6(varray[]);
  opt_alias_fun6(darray[]);
  opt_alias_fun6([]);
  opt_alias_fun6(null);

  opt_alias_fun7(tuple(true, false));
  opt_alias_fun7(shape('a' => true, 'b' => false));
  opt_alias_fun7(varray[]);
  opt_alias_fun7(darray[]);
  opt_alias_fun7([]);
  opt_alias_fun7(null);

  $x = varray[];
  inout_fun1(&$x, tuple(true, false));
  $x = varray[];
  inout_fun1(&$x, shape('a' => true, 'b' => false));
  $x = varray[];
  inout_fun1(&$x, varray[]);
  $x = varray[];
  inout_fun1(&$x, darray[]);
  $x = varray[];
  inout_fun1(&$x, []);

  $x = darray[];
  inout_fun2(&$x, tuple(true, false));
  $x = darray[];
  inout_fun2(&$x, shape('a' => true, 'b' => false));
  $x = darray[];
  inout_fun2(&$x, varray[]);
  $x = darray[];
  inout_fun2(&$x, darray[]);
  $x = darray[];
  inout_fun2(&$x, []);

  $x = darray[];
  inout_fun3(&$x, tuple(true, false));
  $x = darray[];
  inout_fun3(&$x, shape('a' => true, 'b' => false));
  $x = darray[];
  inout_fun3(&$x, varray[]);
  $x = darray[];
  inout_fun3(&$x, darray[]);
  $x = darray[];
  inout_fun3(&$x, []);

  $x = [];
  inout_fun4(&$x, tuple(true, false));
  $x = [];
  inout_fun4(&$x, shape('a' => true, 'b' => false));
  $x = [];
  inout_fun4(&$x, varray[]);
  $x = [];
  inout_fun4(&$x, darray[]);
  $x = [];
  inout_fun4(&$x, []);

  $x = varray[];
  opt_inout_fun1(&$x, tuple(true, false));
  $x = varray[];
  opt_inout_fun1(&$x, shape('a' => true, 'b' => false));
  $x = varray[];
  opt_inout_fun1(&$x, varray[]);
  $x = varray[];
  opt_inout_fun1(&$x, darray[]);
  $x = varray[];
  opt_inout_fun1(&$x, []);
  $x = varray[];
  opt_inout_fun1(&$x, null);

  $x = darray[];
  opt_inout_fun2(&$x, tuple(true, false));
  $x = darray[];
  opt_inout_fun2(&$x, shape('a' => true, 'b' => false));
  $x = darray[];
  opt_inout_fun2(&$x, varray[]);
  $x = darray[];
  opt_inout_fun2(&$x, darray[]);
  $x = darray[];
  opt_inout_fun2(&$x, []);
  $x = darray[];
  opt_inout_fun2(&$x, null);

  $x = darray[];
  opt_inout_fun3(&$x, tuple(true, false));
  $x = darray[];
  opt_inout_fun3(&$x, shape('a' => true, 'b' => false));
  $x = darray[];
  opt_inout_fun3(&$x, varray[]);
  $x = darray[];
  opt_inout_fun3(&$x, darray[]);
  $x = darray[];
  opt_inout_fun3(&$x, []);
  $x = darray[];
  opt_inout_fun3(&$x, null);

  $x = [];
  opt_inout_fun4(&$x, tuple(true, false));
  $x = [];
  opt_inout_fun4(&$x, shape('a' => true, 'b' => false));
  $x = [];
  opt_inout_fun4(&$x, varray[]);
  $x = [];
  opt_inout_fun4(&$x, darray[]);
  $x = [];
  opt_inout_fun4(&$x, []);
  $x = [];
  opt_inout_fun4(&$x, null);

  set_error_handler('handler');

  try {
    throws1(darray[]);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }

  try {
    throws2(darray[]);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }

  try {
    $x = varray[];
    throws3(&$x);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
}


<<__EntryPoint>>
function main_type_hints() {
test();
}
