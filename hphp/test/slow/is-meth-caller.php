<?hh

class Foo { <<__DynamicallyCallable>> function bar() {} }

<<__EntryPoint>>
function main() {
  $mc = meth_caller(Foo::class, 'bar');
  $dmc = hh\dynamic_meth_caller(__hhvm_intrinsics\launder_value(Foo::class), __hhvm_intrinsics\launder_value('bar'));

  $mc2 = __hhvm_intrinsics\launder_value($mc);
  $dmc2 = __hhvm_intrinsics\launder_value($dmc);

  var_dump(hh\is_meth_caller($mc), hh\is_meth_caller($dmc));
  var_dump(hh\is_meth_caller($mc2), hh\is_meth_caller($dmc2));

  var_dump(hh\meth_caller_get_class($mc), hh\meth_caller_get_class($dmc));
  var_dump(hh\meth_caller_get_class($mc2), hh\meth_caller_get_class($dmc2));

  var_dump(hh\meth_caller_get_method($mc), hh\meth_caller_get_method($dmc));
  var_dump(hh\meth_caller_get_method($mc2), hh\meth_caller_get_method($dmc2));
}
