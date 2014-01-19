<?php

error_reporting(E_ALL);

$a = unpack('C2', "\0224V");
$b = array(1 => 18, 2 => 52);
debug_zval_dump($a, $b);
$k = array_keys($a);
$l = array_keys($b);
debug_zval_dump($k, $l);
$i=$k[0];
var_dump($a[$i]);
$i=$l[0];
var_dump($b[$i]);
?>