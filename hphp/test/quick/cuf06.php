<?hh

function handler($a, $b) {
  var_dump($a, $b);
}

function foo() {}

function test() {
  call_user_func_array('foo', varray[]);
}

function main() {
  test();
  fb_rename_function('foo', 'bar');
  test();
}
<<__EntryPoint>>
function main_entry(): void {

  set_error_handler(fun('handler'));

  main();
}
