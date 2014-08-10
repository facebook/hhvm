<?php

/* Prototype: string stristr ( string $haystack, string $needle );
   Description: Case-insensitive strstr().
*/

echo "*** Testing stristr() function: with unexpected inputs for 'needle' argument ***\n";

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
$inputs =  array (

		  // integer values
/*1*/	  0,
		  1,
		  -2,
		  -PHP_INT_MAX,
		
		  // float values
/*5*/	  10.5,
		  -20.5,
		  10.1234567e10,
		
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

//defining '$pad_length' argument
$pad_length = "20";

// loop through with each element of the $inputs array to test stristr() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  var_dump( stristr("Hello World", $input) );
  $count ++;
}

fclose($file_handle);  //closing the file handle

?>
===DONE===
