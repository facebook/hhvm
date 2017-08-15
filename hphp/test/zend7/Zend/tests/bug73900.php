<?php

$a = new stdClass;
$b = new SplFixedArray(1);
$b[0] = $a;
$c = &$b[0];
var_dump($c);

?>
