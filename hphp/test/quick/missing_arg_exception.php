<?hh
set_error_handler(fun('handler'));
function handler() { throw new Exception; }
function foo(&$r) {}
function test() {
  try {
    foo();
  } catch (Exception $e) {
  }
  var_dump('ok');
}
test();
