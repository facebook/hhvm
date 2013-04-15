<?php

 function test($x) {  apc_store('foo', array('a'.$x, array($x)));  $a = apc_fetch('foo');  $x = array_intersect($a, $a);  var_dump($x);}test('foo');