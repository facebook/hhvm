<?php

$obj = (object) ['foo' => 'bar'];
$foo = 'foo';
$ref =& $foo;
var_dump($obj->$foo);

?>
