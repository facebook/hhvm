<?php

$a = [123, 456];
var_dump($a);
foreach ($a as &$item);
var_dump($a);
$b = array_combine($a, $a);
var_dump($a);
var_dump($b);
