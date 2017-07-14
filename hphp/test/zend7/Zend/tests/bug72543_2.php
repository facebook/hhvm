<?php
$arr = [];
$arr[0] = null;
$ref =& $arr[0];
unset($ref);
$arr[0][$arr[0]] = null;
var_dump($arr);
?>
