<?php
/* Prototype  : mixed prev(array $array_arg)
 * Description: Move array argument's internal pointer to the previous element and return it 
 * Source code: ext/standard/array.c
 */

/*
 * Test prev() when passed:
 * 1. a two-dimensional array
 * 2. a sub-array
 * as $array_arg argument.
 */

echo "*** Testing prev() : usage variations ***\n";

$subarray = array(9,8,7);
end($subarray);

$array_arg = array ($subarray, 'a' => 'z');
end($array_arg);

echo "\n-- Pass a two-dimensional array as \$array_arg --\n";
var_dump(prev($array_arg));
var_dump(prev($array_arg));

echo "\n-- Pass a sub-array as \$array_arg --\n";
var_dump(prev($array_arg[0]));
?>
===DONE===