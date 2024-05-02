<?hh

<<__NEVER_INLINE>>
function baz() :mixed{
}

<<__NEVER_INLINE>>
function bar() :mixed{
  // clear tvBuiltinReturn
  HH\ImplicitContext\soft_run_with(baz<>, 'def');
}

<<__NEVER_INLINE>>
function foo() :mixed{
  bar();
  gc_collect_cycles();
  var_dump(HH\ImplicitContext\_Private\get_implicit_context_debug_info());
}

<<__EntryPoint>>
function main() :mixed{
  HH\ImplicitContext\soft_run_with(foo<>, 'abc');
  var_dump("done");
}
