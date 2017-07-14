<?php

$array = ['a', 'b', 'c', 'd'];

foreach ($array as &$a) {
}

var_dump($array);

var_dump(array_values($array));
var_dump($a);

var_dump(array_reverse($array));

?>
