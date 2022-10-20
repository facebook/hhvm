<?hh

<<__NEVER_INLINE>>
function baz() {
}

<<__NEVER_INLINE>>
function bar() {
  // clear tvBuiltinReturn
  HH\ImplicitContext\soft_run_with(baz<>, 'def');
}

<<__NEVER_INLINE>>
function foo() {
  bar();
  gc_collect_cycles();
  var_dump(HH\ImplicitContext\_Private\get_implicit_context_memo_key());
}

<<__EntryPoint>>
function main() {
  HH\ImplicitContext\soft_run_with(foo<>, 'abc');
  var_dump("done");
}
