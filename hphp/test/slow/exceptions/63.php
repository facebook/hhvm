<?hh
function test() {
  for ($i = 0; $i < 4000; $i++) {
    try {
      call_user_func(fun('bar'));
    }
    catch (Exception $e) {
    }
  }
  var_dump('ok');
}
function bar() {
  throw new Exception;
}


<<__EntryPoint>>
function main_63() {
ini_set('memory_limit','18M');
test();
}
