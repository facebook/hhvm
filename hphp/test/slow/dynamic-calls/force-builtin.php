<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function LV($x) { return __hhvm_intrinsics\launder_value($x); }

function wrap($f) {
  try {
    $f()();
  } catch (Exception $e) {
    echo "Caught: ".$e->getMessage()."\n";
  }
}

<<__DynamicallyCallable>>
function foo() { var_dump(__FUNCTION__); }
function bar() { var_dump(__FUNCTION__); }
<<__DynamicallyCallable>>
function reified_fun<reify T>() { var_dump(__FUNCTION__); }
<<__DynamicallyCallable>>
function soft_reified_fun<<<__Soft>> reify T>() { var_dump(__FUNCTION__); }

abstract class Cls {
  <<__DynamicallyCallable>>
  static function foo() { var_dump(__METHOD__); }
  static function bar() { var_dump(__METHOD__); }
  protected static function prot() { var_dump(__METHOD__); }
  private static function priv() { var_dump(__METHOD__); }
  function inst() { var_dump(__METHOD__); }
  abstract static function abstr();
  <<__DynamicallyCallable>>
  static function reified_meth<reify T>() { var_dump(__METHOD__); }
  <<__DynamicallyCallable>>
  static function soft_reified_meth<<<__Soft>> reify T>() {
    var_dump(__METHOD__);
  }
}

<<__EntryPoint>>
function force_builtin_main() {
  foreach (vec['foo', 'bar', 'reified_fun', 'soft_reified_fun'] as $f) {
    wrap(() ==> HH\dynamic_fun_force(LV($f)));
  }

  $meths = vec[
    'foo',
    'bar',
    'prot',
    'priv',
    'inst',
    'abstr',
    'reified_meth',
    'soft_reified_meth',
  ];
  foreach ($meths as $m) {
    wrap(() ==> HH\dynamic_class_meth_force(Cls::class, LV($m)));
  }
}
