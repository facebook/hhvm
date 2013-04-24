<?php

var_dump(array_slice(range(1, 3), 0, NULL, 1));
var_dump(array_slice(range(1, 3), 0, 0, 1));
var_dump(array_slice(range(1, 3), 0, NULL));
var_dump(array_slice(range(1, 3), 0, 0));

var_dump(array_slice(range(1, 3), -1, 0));
var_dump(array_slice(range(1, 3), -1, 0, 1));
var_dump(array_slice(range(1, 3), -1, NULL));
var_dump(array_slice(range(1, 3), -1, NULL, 1));


$a = 'foo';
var_dump(array_slice(range(1, 3), 0, $a));
var_dump(array_slice(range(1, 3), 0, $a));
var_dump($a);

?>
