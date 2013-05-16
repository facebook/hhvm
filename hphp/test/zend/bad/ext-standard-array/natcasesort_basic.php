<?php
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of natcasesort()
 */

echo "*** Testing natcasesort() : basic functionality ***\n";

$array = array ('A01', 'a1', 'b10',  'a01', 'b01');
echo "\n-- Before sorting: --\n";
var_dump($array);

echo "\n-- After Sorting: --\n";
var_dump(natcasesort($array));
var_dump($array);

echo "Done";
?>