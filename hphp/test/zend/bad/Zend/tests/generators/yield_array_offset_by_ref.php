<?php

function &gen(array &$array) {
    yield $array[0];
}

$array = [1, 2, 3];
$gen = gen($array);
foreach ($gen as &$val) {
    $val *= -1;
}
var_dump($array);

?>