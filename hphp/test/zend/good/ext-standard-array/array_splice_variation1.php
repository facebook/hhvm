<?php
/* 
 * proto array array_splice(array input, int offset [, int length [, array replacement]])
 * Function is implemented in ext/standard/array.c
*/ 


echo "test behaviour when input array is in a reference set\n";

$input_array=array (array(1,2));
$input_array[]=&$input_array[0];
var_dump (array_splice ($input_array[0],1,1));
var_dump ($input_array);

echo "Test behaviour of input arrays containing references \n";
/*
 *  There are three regions to test:, before cut, the cut and after the cut.
 *  For reach we check a plain value, a reference value with integer key and a
 *  reference value with a string key.
 */
$numbers=array(0,1,2,3,4,5,6,7,8,9,10,11,12);
$input_array=array(0,1,&$numbers[2],"three"=>&$numbers[3],4,&$numbers[5],"six"=>&$numbers[6],7,&$numbers[8],"nine"=>&$numbers[9]);
var_dump (array_splice ($input_array,4,3));
var_dump ($input_array);

echo "Test behaviour of replacement array containing references \n";

$three=3;
$four=4;
$input_array=array (0,1,2);
$b=array(&$three,"fourkey"=>&$four);
array_splice ($input_array,-1,1,$b);
var_dump ($input_array);

echo "Test behaviour of replacement which is part of reference set \n";

$int=3;
$input_array=array (1,2);
$b=&$int;

array_splice ($input_array,-1,1,$b);
var_dump ($input_array);
echo "Done\n";
?>