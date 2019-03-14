<?php
/* Prototype  : string str_pad  ( string $input  , int $pad_length  [, string $pad_string  [, int $pad_type  ]] )
 * Description: Pad a string to a certain length with another string
 * Source code: ext/standard/string.c
*/

/* Test str_pad() function: with unexpected inputs for '$pad_length' 
 *  and expected type for '$input'
*/

echo "*** Testing str_pad() function: with unexpected inputs for 'pad_length' argument ***\n";

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

// array with different values for $input
$pad_lengths =  array (

		  // integer values
/*1*/	  0,
		  1,
		  -2,
		  255,
		
		  // float values
/*5*/	  10.5,
		  -20.5,
		  10.12345e2,
		
		  // array values
/*8*/	  array(),
		  array(0),
		  array(1, 2),
		
		  // boolean values
/*11*/	  true,
		  false,
		  TRUE,
		  FALSE,
		
		  // null vlaues
/*15*/	  NULL,
		  null,
		
		  // objects
/*17*/	  new sample(),
		
		  // resource
/*18*/	  $file_handle,
		
		  // undefined variable
/*19*/	  @$undefined_var,
		
		  // unset variable
/*20*/	  @$unset_var
);

//defining '$input' argument
$input = "Test string";

// loop through with each element of the $pad_lengths array to test str_pad() function
$count = 1;
foreach($pad_lengths as $pad_length) {
  echo "-- Iteration $count --\n";
  try { var_dump( str_pad($input, $pad_length) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle

?>
===DONE===
