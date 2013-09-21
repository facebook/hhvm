<?php
echo "*** test key_exists() by calling it with its expected arguments ***\n";
$a = array('bar' => 1, 'foo' => array('bar' => 2, 'baz' => 3));  
var_dump(key_exists('baz', $a));
var_dump(key_exists('baz', $a['foo']));