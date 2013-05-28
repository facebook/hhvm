<?php

$x = array('x' => 'y');
$a = array('a1' => &$x, 'a2' => &$x);
$b = array('a1' => array(1,2), 'a2' => array(3,4));
var_dump(array_merge_recursive($a, $b));
