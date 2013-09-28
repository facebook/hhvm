<?php
/* Prototype  : array each(array $arr)
 * Description: Return the currently pointed key..value pair in the passed array,
 * and advance the pointer to the next element 
 * Source code: Zend/zend_builtin_functions.c
 */

/*
 * Test behaviour of each() when:
 * 1. Passed an array made up of referenced variables
 * 2. Passed an array as $arr argument by reference
 */

echo "*** Testing each() : usage variations ***\n";

echo "\n-- Array made up of referenced variables: --\n";
$val1 = 'foo';
$val2 = 'bar';

$arr1 = array('one' => &$val1, &$val2);

echo "-- Call each until at the end of the array: --\n";
var_dump( each($arr1) );
var_dump( each($arr1) );
var_dump( each($arr1) );

echo "Done";
?>
