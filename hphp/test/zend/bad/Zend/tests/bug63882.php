<?php
class Test { public $x = 5; }

$testobj1 = new Test;
$testobj2 = new Test;
$testobj1->x = $testobj1;
$testobj2->x = $testobj2;

var_dump($testobj1 == $testobj2);
?>