<?php

var_dump(preg_replace_callback());
var_dump(preg_replace_callback(1));
var_dump(preg_replace_callback(1,2));
var_dump(preg_replace_callback(1,2,3));
var_dump(preg_replace_callback(1,2,3,4));
$a = 5;
var_dump(preg_replace_callback(1,2,3,4,$a));
$a = "";
var_dump(preg_replace_callback("","","","",$a));
$a = array();
var_dump(preg_replace_callback($a,$a,$a,$a,$a));

echo "Done\n";
?>