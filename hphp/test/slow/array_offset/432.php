<?php

function f(&$elem) {
$elem = 44;
}
$arr = array();
$arr[PHP_INT_MAX-1] = 1;
$arr[PHP_INT_MAX] = 2;
var_dump($arr);
f($arr[]);
var_dump($arr);
unset($arr[PHP_INT_MAX]);
unset($arr[PHP_INT_MAX-1]);
f($arr[]);
var_dump($arr);
