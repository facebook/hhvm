<?php

/* Prototype  : mixed count_chars  ( string $string  [, int $mode  ] )
 * Description: Return information about characters used in a string
 * Source code: ext/standard/string.c
*/

echo "*** Testing count_chars() function: with unexpected inputs for 'mode' argument ***\n";

//get an unset variable
$unset_var = 'string_val';
unset($unset_var);

//defining a class
class sample  {
}

// array with different values for $input
$inputs =  array (

			  // integer values
/* 1 */		  0,
			  1,
			  255,
			  2147483647,
		      -2147483648,
			
			  // float values
/* 6 */		  0.0,
			  1.3,
			  10.5,
			  -20.5,
			  10.1234567e10,
			
			  // array values
/* 11 */	  array(),
			  array(1, 2, 3, 4, 5, 6, 7, 8, 9),
			
			  // boolean values
/* 14 */	  true,
			  false,
			  TRUE,
			  FALSE,
			
			  // null values
/* 18 */	  NULL,
			  null,
			  
			  // string values
/* 20 */	  "ABCD",
			  'abcd',
			  "1ABC",
			  "5ABC",
			  
			  // objects
/* 24 */ 	  new sample(),
			
			   // undefined variable
/* 25 */	  @$undefined_var,
			
			  // unset variable
/* 26 */	  @$unset_var
);

// loop through with each element of the $inputs array to test count_chars() function
// with unexepcted values for the 'mode' argument
$count = 1;
$string = "Return information about characters used in a string";
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  // only list characters with a frequency > 0
  var_dump(is_array(count_chars($string, $input)));
  $count ++;
}


?>
===DONE===
