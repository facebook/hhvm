<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo() {}

<<__DynamicallyCallable>>
function bar() {}

class Cls {
  public static function foo() {}

  <<__DynamicallyCallable>>
  public static function bar() {}
}

<<__EntryPoint>>
function main_exit() {
  $foo = 'foo';
  $bar = 'bar';
  $cls = 'Cls';
  var_dump(HH\dynamic_fun_force($foo));
  var_dump(HH\dynamic_fun_force($bar));
  var_dump(HH\dynamic_class_meth_force($cls, $foo));
  var_dump(HH\dynamic_class_meth_force($cls, $bar));
}
