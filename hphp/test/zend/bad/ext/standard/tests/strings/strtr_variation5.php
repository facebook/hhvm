<?php
/* Prototype  : string strtr(string $str, string $from[, string $to]);
                string strtr(string $str, array $replace_pairs);
 * Description: Translates characters in str using given translation tables
 * Source code: ext/standard/string.c
*/

/* Test strtr() function: with unexpected inputs for 'str' 
 *  and expected type for 'from' & 'to' arguments 
*/

echo "*** Testing strtr() function: with unexpected inputs for 'str' ***\n";

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

// array with different values
$strings =  array (

		  // integer values
/*1*/	  0,
		  1,
		  -2,
		
		  // float values
/*4*/	  10.5,
		  -20.5,
		  10.1234567e10,
		
		  // array values
/*7*/	  array(),
		  array(0),
		  array(1, 2),
		
		  // boolean values
/*10*/	  true,
		  false,
		  TRUE,
		  FALSE,
		
		  // null vlaues
/*14*/	  NULL,
		  null,
		
		  // objects
/*16*/	  new sample(),
		
		  // resource
/*17*/	  $file_handle,
		
		  // undefined variable
/*18*/	  @$undefined_var,
		
		  // unset variable
/*19*/	  @$unset_var
);

//defining 'from' argument
$from = "012atm";

//defining 'to' argument
$to = "atm012";

// loop through with each element of the $strings array to test strtr() function
$count = 1;
for($index = 0; $index < count($strings); $index++) {
  echo "-- Iteration $count --\n";
  $str = $strings[$index];
  var_dump( strtr($str, $from, $to) );
  $count ++;
}

fclose($file_handle);  //closing the file handle

?>
===DONE===
