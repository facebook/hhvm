<?hh
function test() :mixed{
  for ($i = 0; $i < 4000; $i++) {
    try {
      call_user_func(bar<>);
    }
    catch (Exception $e) {
    }
  }
  var_dump('ok');
}
function bar() :mixed{
  throw new Exception;
}


<<__EntryPoint>>
function main_63() :mixed{
ini_set('memory_limit','18M');
test();
}
