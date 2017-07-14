<?php
$array = array("test" => 1);

$a = "lest";
var_dump($array[$a]);
$a[0] = "f";
var_dump($array[$a]);
$a[0] = "t";
var_dump($array[$a]);
?>
