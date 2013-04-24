<?php
/* Prototype  : int array_push(array $stack, mixed $var [, mixed $...])
 * Description: Pushes elements onto the end of the array 
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of array_push with indexed and associative arrays
 */

echo "*** Testing array_push() : basic functionality ***\n";

$array = array ('zero', 'one', 'two');
$var1 = 'three';
$var2 = 'four';

echo "\n-- Push values onto an indexed array --\n";
var_dump(array_push($array, $var1, $var2));
var_dump($array);

$array_assoc = array ('one' => 'un', 'two' => 'deux');

echo "\n-- Push values onto an associative array --\n";
var_dump(array_push($array_assoc, $var1, $var2));
var_dump($array_assoc);

echo "Done";
?>
