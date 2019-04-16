<?php
/* Prototype  : array array_intersect_uassoc(array arr1, array arr2 [, array ...], callback key_compare_func)
 * Description: Computes the intersection of arrays with additional index check, compares indexes by a callback function
 * Source code: ext/standard/array.c
 */

echo "*** Testing array_intersect_uassoc() : usage variation ***\n";

//Initialize variables
$ref_var = 'a';
$array1 = array('a', $ref_var);
$array2 = array('a' => 1, &$ref_var);

echo "\n-- Testing array_intersect_uassoc() function with referenced variable \$ref_var has value '$ref_var' --\n";
var_dump( array_intersect_uassoc($array1, $array2, ($a, $b) ==> strcasecmp((string)$a, (string)$b)) );

// re-assign reference variable to different value
$ref_var = 10;
echo "\n-- Testing array_intersect_uassoc() function with referenced variable \$ref_var value changed to $ref_var --\n";
var_dump( array_intersect_uassoc($array1, $array2, ($a, $b) ==> strcasecmp((string)$a, (string)$b)) );

echo "===DONE===\n";
