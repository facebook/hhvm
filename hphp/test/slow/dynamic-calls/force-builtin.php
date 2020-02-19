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

class Cls {
  <<__DynamicallyCallable>>
  static function foo() { var_dump(__METHOD__); }
  static function bar() { var_dump(__METHOD__); }
  protected static function prot() { var_dump(__METHOD__); }
  private static function priv() { var_dump(__METHOD__); }
  function inst() { var_dump(__METHOD__); }
}

<<__EntryPoint>>
function force_builtin_main() {
  foreach (vec['foo', 'bar'] as $f) {
    wrap(() ==> HH\dynamic_fun_force(LV($f)));
  }

  foreach (vec['foo', 'bar', 'prot', 'priv', 'inst'] as $m) {
    wrap(() ==> HH\dynamic_class_meth_force(Cls::class, LV($m)));
  }
}
