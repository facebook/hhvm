<?php
$a = new stdClass();
$x = "";
$a->$x = "string('')";
var_dump($a);
$a->{"\0"} = 42;
var_dump($a);
?>
