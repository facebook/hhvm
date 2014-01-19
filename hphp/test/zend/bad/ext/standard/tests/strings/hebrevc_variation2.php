<?php

/* Prototype  : string hebrevc  ( string $hebrew_text  [, int $max_chars_per_line  ] )
 * Description: Convert logical Hebrew text to visual text
 * Source code: ext/standard/string.c
*/

echo "*** Testing hebrevc() function: with unexpected inputs for 'max_chars_per_line' argument ***\n";

//get an unset variable
$unset_var = 'string_val';
unset($unset_var);

//defining a class
class sample  {
  public function __toString() {
    return "sample object";
  } 
}

//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $max_chars_per_line
$inputs =  array (

		  // integer values
/*1*/	  0,
		  1,
		  255,
		  256,
	      2147483647,
		  -2147483648,
		
		  // float values
/*7*/	  10.5,
		  -20.5,
		  10.1234567e5,
		
		  // array values
/*10*/	  array(),
		  array(0),
		  array(1, 2),
		
		  // boolean values
/*13*/	  true,
		  false,
		  TRUE,
		  FALSE,
		
		  // null values
/*17*/	  NULL,
		  null,
		  
		  // string values
/*19*/	  "abc",
		  'abc',
		  "3abc",
		  "0abc",
		  "0x3",
		
		  // objects
/*24*/	  new sample(),
		
		  // resource
/*25*/	  $file_handle,
		
		  // undefined variable
/*26*/	  @$undefined_var,
		
		  // unset variable
/*27*/	  @$unset_var
);

// loop through with each element of the $texts array to test hebrevc() function
$count = 1;

$hebrew_text = "The hebrevcc function converts logical Hebrew text to visual text.\nThis function is similar to hebrevc() with the difference that it converts newlines (\n) to '<br>\n'.\nThe function tries to avoid breaking words.\n";

foreach($inputs as $max_chars_per_line) {
  echo "-- Iteration $count --\n";
  var_dump( hebrevc($hebrew_text, $max_chars_per_line) );
  $count ++;
}

fclose($file_handle);  //closing the file handle

?>
===DONE===