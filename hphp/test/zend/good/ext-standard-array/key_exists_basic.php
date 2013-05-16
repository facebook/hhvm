<?php
echo "*** test key_exists() by calling it with its expected arguments ***\n";
$a = array('bar' => 1);  
var_dump(key_exists('bar', $a));
var_dump(key_exists('foo', $a));