<?php

function rmv($a, $b) {
 unset($a[$b]);
 return $a;
 }
$a = array('foo');
$b = array();
var_dump(rmv($a, $b));
