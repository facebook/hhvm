<?php

$a = array(array('one'),array('one'));
function test($v, $k) {
 }
array_walk_recursive($a, 'foo');
function foo($a) {
 var_dump($a);
 }
