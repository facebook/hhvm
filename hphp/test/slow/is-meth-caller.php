<?hh

class Foo { <<__DynamicallyCallable>> function bar() :mixed{} }

<<__EntryPoint>>
function main() :mixed{
  $mc = meth_caller(Foo::class, 'bar');
  $dmc = HH\dynamic_meth_caller(__hhvm_intrinsics\launder_value(Foo::class), __hhvm_intrinsics\launder_value('bar'));

  $mc2 = __hhvm_intrinsics\launder_value($mc);
  $dmc2 = __hhvm_intrinsics\launder_value($dmc);

  var_dump(HH\is_meth_caller($mc), HH\is_meth_caller($dmc));
  var_dump(HH\is_meth_caller($mc2), HH\is_meth_caller($dmc2));

  var_dump(HH\meth_caller_get_class($mc), HH\meth_caller_get_class($dmc));
  var_dump(HH\meth_caller_get_class($mc2), HH\meth_caller_get_class($dmc2));

  var_dump(HH\meth_caller_get_method($mc), HH\meth_caller_get_method($dmc));
  var_dump(HH\meth_caller_get_method($mc2), HH\meth_caller_get_method($dmc2));
}
