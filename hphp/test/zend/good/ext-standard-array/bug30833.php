<?php

$foo = array('abc', '0000');
var_dump($foo);

$count = array_count_values( $foo );
var_dump($count);

var_dump($foo);

echo "Done\n";
?>