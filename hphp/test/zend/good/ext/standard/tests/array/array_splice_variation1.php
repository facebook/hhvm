<?php
/*
 * proto array array_splice(array input, int offset [, int length [, array replacement]])
 * Function is implemented in ext/standard/array.c
*/


echo "test behaviour when input array is in a reference set\n";

$inner_array = array(1, 2);
$input_array= array(&$inner_array, &$inner_array);
var_dump (array_splice (&$inner_array,1,1));
var_dump ($input_array);

echo "Test behaviour of input arrays containing references \n";
/*
 *  There are three regions to test:, before cut, the cut and after the cut.
 *  For reach we check a plain value, a reference value with integer key and a
 *  reference value with a string key.
 */
$numbers=array(0,1,2,3,4,5,6,7,8,9,10,11,12);
$n2 = 2;
$n3 = 3;
$n5 = 5;
$n6 = 6;
$n8 = 8;
$n9 = 9;
$input_array=array(0,1,&$n2,"three"=>&$n3,4,&$n5,"six"=>&$n6,7,&$n8,"nine"=>&$n9);
var_dump (array_splice (&$input_array,4,3));
var_dump ($input_array);

echo "Test behaviour of replacement array containing references \n";

$three=3;
$four=4;
$input_array=array (0,1,2);
$b=array(&$three,"fourkey"=>&$four);
array_splice (&$input_array,-1,1,$b);
var_dump ($input_array);

echo "Test behaviour of replacement which is part of reference set \n";

$int=3;
$input_array=array (1,2);

array_splice (&$input_array,-1,1,$int);
var_dump ($input_array);
echo "Done\n";
