<?php
/* 
 * proto array array_splice(array input, int offset [, int length [, array replacement]])
 * Function is implemented in ext/standard/array.c
*/ 

$array=array(0,1,2);
var_dump (array_splice($array,1,1,3,4,5,6,7,8,9));
var_dump ($array);
echo "Done\n";
?>