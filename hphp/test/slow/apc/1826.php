<?php

function test($x) {
  apc_store('foo', array('a'.$x, array($x)));
  $a = apc_fetch('foo');
  $x = array_intersect($a, $a);
  var_dump($x);
}


// disable array -> "Array" conversion notice
<<__EntryPoint>>
function main_1826() {
error_reporting(error_reporting() & ~E_NOTICE);
test('foo');
}
