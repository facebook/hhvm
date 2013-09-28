<?php

$a = array('a', 'b', 'c');
$last = array_shift ($a);
$a[] = 'a';
var_dump($a);

$a = array('a' => 1, 'b' => 2, 'c' => 3);
$last = array_shift ($a);
$a[] = 'a';
var_dump($a);

?>