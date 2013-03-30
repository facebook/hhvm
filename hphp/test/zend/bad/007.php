<?php

var_dump(each());
$var = 1;
var_dump(each($var));
$var = "string";
var_dump(each($var));
$var = array(1,2,3);
var_dump(each($var));
$var = array("a"=>1,"b"=>2,"c"=>3);
var_dump(each($var));

$a = array(1);
$a [] =&$a[0];

var_dump(each($a));


echo "Done\n";
?>