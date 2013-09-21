<?php

$array = new SplFixedArray(5);

$obj = new stdClass;
$obj->prop = 'value';

$array[0] = 'foo';
$array[2] = 42;
$array[3] = $obj;
$array[4] = range(1, 5);

$ser = serialize($array);
echo "$ser\n";
$unser = unserialize($ser);

printf("count: %d\n", count($unser));
printf("getSize(): %d\n", $unser->getSize());

var_dump($unser[0], $unser[1], $unser[2], $unser[3], $unser[4]);

$unser[4] = 'quux';
var_dump($unser[4]);

?>