<?hh

function handler($a, $b) :mixed{
  var_dump($a, $b);
}

function foo() :mixed{}

function test() :mixed{
  call_user_func_array('foo', vec[]);
}

function main() :mixed{
  test();
  fb_rename_function('foo', 'bar');
  test();
}
<<__EntryPoint>>
function main_entry(): void {

  set_error_handler(handler<>);

  main();
}
