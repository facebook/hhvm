<?php
$ai = new ArrayIterator(array(0,1));

var_dump($ai->count());

$ii = new IteratorIterator($ai);

var_dump($ii->count());
?>