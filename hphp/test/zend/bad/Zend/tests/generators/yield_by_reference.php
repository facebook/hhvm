<?php

function &iter(array &$array) {
    foreach ($array as $key => &$value) {
        yield $key => $value;
    }
}

$array = [1, 2, 3];
$iter = iter($array);
foreach ($iter as &$value) {
    $value *= -1;
}
var_dump($array);

$array = [1, 2, 3];
foreach (iter($array) as &$value) {
    $value *= -1;
}
var_dump($array);

?>