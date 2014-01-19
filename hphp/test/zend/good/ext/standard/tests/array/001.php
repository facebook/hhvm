<?php
require(dirname(__FILE__) . '/data.inc');
/*
** Create sample arrays
** Test alpha, numeric (decimal, hex, octal) and special data
**
**
*/

/* Helper function to build testing arrays */
function make_nested_array ($depth, $breadth, $function = NULL, $args = array ()) {
    for ($x = 0; $x < $breadth; ++$x) {
        if (NULL === $function) {
            $array = array (0);
        } else {
            $array = array (call_user_func_array ($function, $args));
        }
        for ($y = 1; $y < $depth; ++$y) {
            $array[0] = array ($array[0]);
        }
        $temp[$x] = $array;
    }
    return $temp;
}

/* Nested array */
$data2 = make_nested_array (3, 3);
$data = array_merge($data, $data2);

var_dump ($data);

function echo_kv ($value, $key) {
    var_dump ($key);
    var_dump ($value);
}

echo " -- Testing array_walk() -- \n";
array_walk ($data, 'echo_kv');

?>