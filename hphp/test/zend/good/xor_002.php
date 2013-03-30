<?php

$s = "123";
$s1 = "234";
var_dump(bin2hex($s ^ $s1));

$s = "1235";
$s1 = "234";
var_dump(bin2hex($s ^ $s1));

$s = "some";
$s1 = "test";
var_dump(bin2hex($s ^ $s1));

$s = "some long";
$s1 = "test";
var_dump(bin2hex($s ^ $s1));

$s = "some";
$s1 = "test long";
var_dump(bin2hex($s ^ $s1));

$s = "some";
$s ^= "test long";
var_dump(bin2hex($s));

echo "Done\n";
?>