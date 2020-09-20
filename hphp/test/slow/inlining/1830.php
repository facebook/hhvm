<?hh

function foo($e, $m) {
  $_REQUEST['_foo'] = $e;
  $_REQUEST['_bar'] = $m;
  return $e;
}
function test($x) {
  return foo('a', $x);
}

<<__EntryPoint>>
function main_1830() {
var_dump(test('b'));
}
