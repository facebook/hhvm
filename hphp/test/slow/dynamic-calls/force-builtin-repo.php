<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo() :mixed{}

<<__DynamicallyCallable>>
function bar() :mixed{}

class Cls {
  public static function foo() :mixed{}

  <<__DynamicallyCallable>>
  public static function bar() :mixed{}
}

<<__EntryPoint>>
function main_exit() :mixed{
  $foo = 'foo';
  $bar = 'bar';
  $cls = 'Cls';

  $count = __hhvm_intrinsics\apc_fetch_no_check('count');
  if ($count === false) $count = 0;
  if ($count < 4) {
    ++$count;
    apc_store('count', $count);
    echo "====================== $count =======================\n";
    switch ($count) {
      case 1:
        var_dump(HH\dynamic_fun_force($foo));
        break;
      case 2:
        var_dump(HH\dynamic_fun_force($bar));
        break;
      case 3:
        var_dump(HH\dynamic_class_meth_force($cls, $foo));
        break;
      case 4:
        var_dump(HH\dynamic_class_meth_force($cls, $bar));
        break;
      default:
        break;
    }
  }
}
