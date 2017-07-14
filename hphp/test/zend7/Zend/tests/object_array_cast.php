<?php

$arr = [1, 2, 3];
var_dump((object) (array) $arr);
var_dump($arr);

$obj = (object) [1, 2, 3];
var_dump((array) (object) $obj);
var_dump($obj);

?>
