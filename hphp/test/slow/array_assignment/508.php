<?php

$a = array(1=>'main', 2=>'sub');
$b = $a;
var_dump(array_pop($b));
print_r($a);
var_dump(array_shift($b));
print_r($a);
