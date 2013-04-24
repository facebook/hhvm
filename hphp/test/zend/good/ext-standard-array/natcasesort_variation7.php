<?php
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass natcasesort() an infinitely recursive array to test how it is re-ordered
 */

echo "*** Testing natcasesort() : usage variations ***\n";

$array = array (1, 3.00, 'zero', '2');
$array[] = &$array;
var_dump($array);

var_dump(@natcasesort($array));
var_dump($array);

echo "Done";
?>