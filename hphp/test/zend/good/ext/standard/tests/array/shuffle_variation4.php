<?php
/* Prototype  : bool shuffle(array $array_arg)
 * Description: Randomly shuffle the contents of an array 
 * Source code: ext/standard/array.c
*/

/*
* Test behaviour of shuffle() function when associative arrays 
* having different types of values, are passed to 'array_arg' argument
*/

echo "*** Testing shuffle() : associative arrays with diff types of values ***\n";

// initialise different arrays
$array_arg = array(
       // array with positive int values
/*1*/  array("zero" => 0, 1 => 1, "two" => 2, "max_int" => 2147483647 ),

       // array with negative int values
       array("minus_one" => -1, 'minus_two' => -2, "min_int" => -2147483647 ),

       // array with positive float values
/*3*/  array("float1" => 0.23, 'float2' => 1.34, "exp1" => 0e2, 'exp2' => 200e-2, "exp3" =>  10e0),

       // array with negative float values
       array(-0.23 => -0.23, -1.34 => -1.34, -200e-2 => -200e-2, -30 => -30e0, -2147473649.80),

       // array with single and double quoted strings
/*5*/  array('1' => 'one', "str1" => "123numbers", '' => 'hello\tworld', "" => "hello world\0", "12.34floatnum"),

       // array with bool values
       array('1' => TRUE, "1" => TRUE, "0" => FALSE, '0' => FALSE),

       // array with positive hexa values
/*7*/  array("hex1" => 0x123, 'hex2' => 0xabc, "hex\t3" => 0xABC, "hex\04" => 0xAb1),

       // array with negative hexa values
       array(NULL => -0x123, "NULL" => -0xabc, "-ABC" => -0xABC, -0xAB1 => -0xAb1),

       // array with positive octal values
/*9*/  array(0123 => 0123, "02348" => 02348, '034' => 034, 00 => 00),

       // array with negative octal values
       array(-0123 => -0123, "-02348" => -02348, '-034' => -034),

       // array with null values
/*11*/ array(NULL => NULL, "null" => NULL, "NULL" => NULL)

);

// looping to test shuffle() with each sub-array in the $array_arg array
echo "\n*** Testing shuffle() with arrays having different types of values ***\n";
$counter = 1;
foreach($array_arg as $arr) {
  echo "\n-- Iteration $counter --\n";
  var_dump( shuffle($arr) );  
  echo "\nThe output array is:\n";
  var_dump( $arr ); 
  $counter++;
}

echo "Done";
?>