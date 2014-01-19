<?php

$a = array("a", "b", "c");

var_dump(key($a));
var_dump(array_pop($a));
var_dump(key($a));      
var_dump(array_pop($a));
var_dump(key($a));      
var_dump(array_pop($a));
var_dump(key($a));      

?>