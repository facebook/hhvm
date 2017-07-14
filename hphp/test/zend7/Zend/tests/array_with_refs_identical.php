<?php

$foo = 42;
$array1 = [&$foo];
$array2 = [$foo];
var_dump($array1 === $array2);

?>
