<?php
echo "--> No arguments:\n";
var_dump(new ArrayObject());

echo "--> Object argument:\n";
$a = new stdClass;
$a->p = 'hello';
var_dump(new ArrayObject($a));

echo "--> Array argument:\n";
var_dump(new ArrayObject(array('key1' => 'val1')));

echo "--> Nested ArrayObject argument:\n";
var_dump(new ArrayObject(new ArrayObject($a)));
?>