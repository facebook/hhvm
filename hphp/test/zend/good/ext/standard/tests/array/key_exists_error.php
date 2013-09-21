<?php

echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

$a = array('bar' => 1);  
var_dump(key_exists());
var_dump(key_exists('foo', $a, 'baz'));

?>