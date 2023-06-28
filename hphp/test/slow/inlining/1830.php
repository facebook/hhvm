<?hh

function foo($e, $m) :mixed{
  $_REQUEST['_foo'] = $e;
  $_REQUEST['_bar'] = $m;
  return $e;
}
function test($x) :mixed{
  return foo('a', $x);
}

<<__EntryPoint>>
function main_1830() :mixed{
var_dump(test('b'));
}
