<?php
$foo = array('foo' => 1, 'bar' => 2, 'test' => 3);
extract($foo);
var_dump($foo, $bar, $test);
?>