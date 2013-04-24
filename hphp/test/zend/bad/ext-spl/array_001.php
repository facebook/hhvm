<?php

$ar = array(0=>0, 1=>1);
$ar = new ArrayObject($ar);

var_dump($ar);

$ar[2] = 2;
var_dump($ar[2]);
var_dump($ar["3"] = 3);

var_dump(array_merge((array)$ar, array(4=>4, 5=>5)));

var_dump($ar["a"] = "a");

var_dump($ar);
var_dump($ar[0]);
var_dump($ar[6]);
var_dump($ar["b"]);

unset($ar[1]);
unset($ar["3"]);
unset($ar["a"]);
unset($ar[7]);
unset($ar["c"]);
var_dump($ar);

$ar[] = '3';
$ar[] = 4;
var_dump($ar);

?>
===DONE===
<?php exit(0); ?>