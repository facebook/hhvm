<?php
$arr = [1];
$ref =& $arr[0];
var_dump($arr[0] + ($arr[0] = 2));
?>
