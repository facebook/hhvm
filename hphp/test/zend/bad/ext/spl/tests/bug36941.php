===ArrayObject===
<?php
$a = new ArrayObject();
$a[] = 1;

$b = clone $a;

var_dump($a[0], $b[0]);
$b[0] = $b[0] + 1;
var_dump($a[0], $b[0]);
$b[0] = 3;
var_dump($a[0], $b[0]);
?>
===ArrayIterator===
<?php
$a = new ArrayIterator();
$a[] = 1;

$b = clone $a;

var_dump($a[0], $b[0]);
$b[0] = $b[0] + 1;
var_dump($a[0], $b[0]);
$b[0] = 3;
var_dump($a[0], $b[0]);
?>
===DONE===