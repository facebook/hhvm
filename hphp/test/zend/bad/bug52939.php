<?php
$ar1 = array("row1" => 2, "row2" => 1);
var_dump(array_multisort($ar1));
var_dump($ar1);

$ar1 = array("row1" => 2, "row2" => 1);
$args = array(&$ar1);
var_dump(call_user_func_array("array_multisort", $args));
var_dump($ar1);

$ar1 = array("row1" => 2, "row2" => 1);
$args = array($ar1);
var_dump(call_user_func_array("array_multisort", $args));
var_dump($ar1);
?>