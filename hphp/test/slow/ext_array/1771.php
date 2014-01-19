<?php

$a = array('foo'=>array('bar'=>1));
function fix(&$v, $k) {
 $v *= 2;
 }
array_walk_recursive($a, 'fix');
var_dump($a['foo']);
