<?php

// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

function test($x) {
  apc_store('foo', array('a'.$x, array($x)));
  $a = apc_fetch('foo');
  $x = array_intersect($a, $a);
  var_dump($x);
}
test('foo');
