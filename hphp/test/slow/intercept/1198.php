<?php

function foo(&$a) {
  var_dump('foo');
  $a = 1;
}
function bar(&$a) {
  var_dump('bar');
  $a = 2;
}
function goo($name, $obj, $params, $data, &$done) {
  return call_user_func_array($data, $params);
}
fb_intercept('foo', 'goo', 'bar');
$a = 0;
foo($a);
var_dump($a);
