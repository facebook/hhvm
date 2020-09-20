<?hh
function handler() { throw new Exception; }
function foo(inout $r) {}
function test() {
  try {
    foo();
  } catch (Exception $e) {
  }
  var_dump('ok');
}
<<__EntryPoint>>
function main_entry(): void {
  set_error_handler(fun('handler'));
  test();
}
