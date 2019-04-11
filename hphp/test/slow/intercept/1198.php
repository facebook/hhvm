<?hh

function foo(&$a) {
  var_dump('foo');
  $a[0] = 1;
}
function bar(&$a) {
  var_dump('bar');
  $a[0] = 2;
}
function goo($name, $obj, $params, $data, &$done) {
  return $data(&$params);
}

<<__EntryPoint>>
function main_1198() {
fb_intercept('foo', 'goo', 'bar');
$a = 0;
foo(&$a);
var_dump($a);
}
