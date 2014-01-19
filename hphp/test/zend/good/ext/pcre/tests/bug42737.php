<?php

$string = chr(13).chr(10);

$array = preg_split('//u', $string, - 1, PREG_SPLIT_NO_EMPTY);

var_dump(array_map('ord', $array));

?>