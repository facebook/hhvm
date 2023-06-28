<?hh
function handle() :mixed{
 throw new Exception;
 }
function foo($a,$b=null) :mixed{
 return $a;
 }
function test1() :mixed{
  if (foo(0)) $a=1;
  $x = new stdClass;
  return $a;
}
function test2() :mixed{
  if (foo(0)) $a=1;
  return $a | new stdClass;
}
function test3() :mixed{
  if (foo(0)) $a=1;
  $x = new stdClass;
  return $a::foo;
}
function test($f) :mixed{
  try {
    $f();
  }
 catch (Exception $e) {
    var_dump($f.':Caught');
  }
}


<<__EntryPoint>>
function main_65() :mixed{
error_reporting(-1);
set_error_handler(handle<>);
test('test1');
test('test2');
test('test3');
set_error_handler(null);
test('test3');
var_dump('not reached');
}
