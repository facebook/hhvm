<?php

$a = ['zero', 'one', 'two'];
unset($a[2]);
$b = $a;
$a[] = 'three';
$b[] = 'three';

var_dump($a === $b);

?>
