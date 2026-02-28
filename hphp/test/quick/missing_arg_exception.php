<?hh
function handler() :mixed{ throw new Exception; }
function foo(inout $r) :mixed{}
function test() :mixed{
  try {
    foo();
  } catch (Exception $e) {
  }
  var_dump('ok');
}
<<__EntryPoint>>
function main_entry(): void {
  set_error_handler(handler<>);
  test();
}
