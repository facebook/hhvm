<?php

$arr = array(2, 1);
$b =& $arr;

list(,$a) = $b;

var_dump($a, $b);

?>