<?php
/* Prototype  : string strval  ( mixed $var  )
 * Description: Get the string value of a variable. 
 * Source code: ext/standard/string.c
 */

echo "*** Testing strval() : error conditions ***\n";

error_reporting(E_ALL ^ E_NOTICE);

class MyClass
{
	// no toString() method defined 
}

$string  = "Hello";
$extra_arg = 10;

//Test strval with one more than the expected number of arguments
echo "\n-- Testing strval() function with more than expected no. of arguments --\n";
var_dump( strval($string, $extra_arg) );

// Testing strval with one less than the expected number of arguments
echo "\n-- Testing strval() function with less than expected no. of arguments --\n";
var_dump( strval() );

// Testing strval with a object which has no toString() method
echo "\n-- Testing strval() function with object which has not toString() method  --\n";
var_dump( strval(new MyClass()) );

?>
===DONE===