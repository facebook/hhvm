<?php
/* Prototype  : mixed key(array $array_arg)
 * Description: Return the key of the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 */

/*
 * Test how key() behaves with muti-dimensional and recursive arrays
 */

echo "*** Testing key() : usage variations ***\n";

echo "\n-- Two Dimensional Array --\n";
$multi_array = array ('zero', array (1, 2, 3), 'two');
echo "Initial Position: ";
var_dump(key(&$multi_array));

echo "Next Position:    ";
next(&$multi_array);
var_dump(key(&$multi_array));

echo "End Position:     ";
end(&$multi_array);
var_dump(key(&$multi_array));

echo "===DONE===\n";
