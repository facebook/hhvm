<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test1() {
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(tuple(true, false));
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(shape('a' => true, 'b' => false));
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(varray[]);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(darray[]);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
}

function test2($a, $b, $c, $d, $e) {
  $a = __hhvm_intrinsics\launder_value($a);
  $b = __hhvm_intrinsics\launder_value($b);
  $c = __hhvm_intrinsics\launder_value($c);
  $d = __hhvm_intrinsics\launder_value($d);
  $e = __hhvm_intrinsics\launder_value($e);

  __hhvm_intrinsics\dummy_varr_or_darr_builtin($a);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin($b);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin($c);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin($d);
  __hhvm_intrinsics\dummy_varr_or_darr_builtin($e);
}

function test3() {
  __hhvm_intrinsics\dummy_array_builtin(tuple(true, false));
  __hhvm_intrinsics\dummy_array_builtin(shape('a' => true, 'b' => false));
  __hhvm_intrinsics\dummy_array_builtin(varray[]);
  __hhvm_intrinsics\dummy_array_builtin(darray[]);
  __hhvm_intrinsics\dummy_array_builtin(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
}

function test4($a, $b, $c, $d, $e) {
  $a = __hhvm_intrinsics\launder_value($a);
  $b = __hhvm_intrinsics\launder_value($b);
  $c = __hhvm_intrinsics\launder_value($c);
  $d = __hhvm_intrinsics\launder_value($d);
  $e = __hhvm_intrinsics\launder_value($e);

  __hhvm_intrinsics\dummy_array_builtin($a);
  __hhvm_intrinsics\dummy_array_builtin($b);
  __hhvm_intrinsics\dummy_array_builtin($c);
  __hhvm_intrinsics\dummy_array_builtin($d);
  __hhvm_intrinsics\dummy_array_builtin($e);
}

class Pile {
  public static $logs = vec[];
}

function handler($errno, $errstr) {
  Pile::$logs[] = $errstr;
}

function process_logs($uniquify = true) {
  $seen = keyset[];
  foreach (Pile::$logs as $log) {
    if ($uniquify && isset($seen[$log])) { continue; }
    $seen[] = $log;
    print($log."\n\n");
  }
  Pile::$logs = vec[];
}

<<__EntryPoint>>
function main_type_hints_builtins() {
  set_error_handler(fun('handler'));

  test1();
  test2(
    tuple(true, false),
    shape('a' => true, 'b' => false),
    varray[],
    darray[],
    __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])
  );
  process_logs();

  test3();
  test4(
    tuple(true, false),
    shape('a' => true, 'b' => false),
    varray[],
    darray[],
    __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])
  );
  process_logs(false);
}
