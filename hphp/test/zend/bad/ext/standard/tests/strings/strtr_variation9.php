<?php
/* Prototype  : string strtr(string $str, string $from[, string $to]);
                string strtr(string $str, array $replace_pairs);
 * Description: Translates characters in str using given translation tables
 * Source code: ext/standard/string.c
*/

/* Test strtr() function: with unexpected inputs for 'str', 'from', 'to' & 'replace_pairs' arguments */

echo "*** Testing strtr() function: with unexpected inputs for all arguments ***\n";

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
$values =  array (

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
		  array(1),
		  array(1, 2),
		  array('color' => 'red', 'item' => 'pen'),
		
		  // boolean values
/*12*/	  true,
		  false,
		  TRUE,
		  FALSE,
		
		  // null vlaues
/*16*/	  NULL,
		  null,
		
		  // objects
/*18*/	  new sample(),
		
		  // resource
/*19*/	  $file_handle,
		
		  // undefined variable
/*20*/	  @$undefined_var,
		
		  // unset variable
/*21*/	  @$unset_var
);

// loop through with each element of the $values array to test strtr() function
$count = 1;
for($index = 0; $index < count($values); $index++) {
  echo "\n-- Iteration $count --\n";
  var_dump( strtr($values[$index], $values[$index], $values[$index]) ); //fn call with three args
  var_dump( strtr($values[$index], $values[$index]) );  //fn call with two args
  $count ++;
}

fclose($file_handle);  //closing the file handle

?>
===DONE===