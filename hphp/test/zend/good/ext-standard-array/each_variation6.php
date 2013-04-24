<?php
/* Prototype  : array each(array $arr)
 * Description: Return the currently pointed key..value pair in the passed array,
 * and advance the pointer to the next element 
 * Source code: Zend/zend_builtin_functions.c
 */

/*
 * Test the position of the internal array pointer after a call to each()
 */

echo "*** Testing each() : usage variations ***\n";

$arr = array('zero', 'one', 'two', 'abc', 'xyz');

echo "\n-- Current position: --\n";
echo key($arr) . " => " . current($arr) . "\n";

echo "\n-- Call to each(): --\n";
var_dump( each($arr) );

echo "\n-- New position: --\n";
echo key($arr) . " => " . current($arr) . "\n";

echo "Done";
?>
