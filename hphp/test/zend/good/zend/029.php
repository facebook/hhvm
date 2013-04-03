<?php

$arr = array(new stdClass);

$arr[0]->a = clone $arr[0];
var_dump($arr);

$arr[0]->b = new $arr[0];
var_dump($arr);

$arr[0]->c = $arr[0]->a;
var_dump($arr);

?>