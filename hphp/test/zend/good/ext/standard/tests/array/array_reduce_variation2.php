<?php
/* Prototype  : mixed array_reduce(array input, mixed callback [, int initial])
 * Description: Iteratively reduce the array to a single value via the callback. 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */

echo "*** Testing array_reduce() : variation - invalid parameters ***\n";


$array = array(1);

var_dump(array_reduce($array, "bogusbogus"));

var_dump(array_reduce("bogusarray", "max"));

var_dump(array_reduce(new stdClass(), "max"));

?>
===DONE===