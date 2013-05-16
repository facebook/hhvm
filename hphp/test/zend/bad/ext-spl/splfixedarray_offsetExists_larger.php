<?php
$ar = new SplFixedArray(3);
$ar[0] = 1;
$ar[1] = 2;
$ar[2] = 3;

var_dump($ar->offsetExists(4));
?>