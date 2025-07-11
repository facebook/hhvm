<?hh

<<__DynamicallyCallable>> function foo() :mixed{ echo __FUNCTION__,"\n"; }

function test() :mixed{
  call_user_func_array(HH\dynamic_fun('foo'), vec[]);
}

function main() :mixed{
  test();
  fb_rename_function('foo', 'bar');
  test();
}
<<__EntryPoint>>
function main_entry(): void {
  main();
}
