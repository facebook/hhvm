<?php

function foo($e, $m) {
  $_REQUEST['_foo'] = $e;
  $_REQUEST['_bar'] = $m;
  return $e;
}
function test($x) {
  return foo('a', $x);
}
var_dump(test('b'));
