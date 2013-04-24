<?php
/* Prototype  : string str_pad  ( string $input  , int $pad_length  [, string $pad_string  [, int $pad_type  ]] )
 * Description: Pad a string to a certain length with another string
 * Source code: ext/standard/string.c
*/

/* Test str_pad() function: with unexpected inputs for '$pad_type' 
 *  and expected type for '$input', '$pad_length' and '$pad_string'
*/

echo "*** Testing str_pad() function: with unexpected inputs for 'pad_type' argument ***\n";

//get an unset variable
$unset_var = 'string_val';
unset($unset_var);

//defining a class
class sample  {
  public function __toString() {
    return "sample object";
  } 
}

// array with different values for $input
$pad_types =  array (

		  // integer values
/*1*/	  0, // == STR_PAD_LEFT
		  1, // == STR_PAD_RIGHT
		  2, // == STR_PAD_BOTH
		  -2, 
		  2147483647,
		  -2147483648,
		  	
		  // float values
/*7*/	  10.5,
		  -20.5,
		  10.1234567e10,
		  
		  // string data
/*10*/	  "abc",
		  "STR_PAD_LEFT",
		  "2",
		  "0x2",
		  "02",
		  
		  // array values
/*15*/	  array(),
		  array(0),
		  array(1, 2),
		
		  // boolean values
/*18*/	  true,
		  false,
		  TRUE,
		  FALSE,
		
		  // null vlaues
/*22*/	  NULL,
		  null,
		
		  // objects
/*24*/	  new sample(),
		
		  // undefined variable
/*25*/	  @$undefined_var,
		
		  // unset variable
/*26*/	  @$unset_var
);

//defining '$input' argument
$input = "Test string";
$pad_length = 20;
$pad_string = "*";

// loop through with each element of the $pad_types array to test str_pad() function
$count = 1;
foreach($pad_types as $pad_type) {
  echo "-- Iteration $count --\n";
  var_dump( str_pad($input, $pad_length, $pad_string, $pad_type) );
  $count ++;
}

?>
===DONE===