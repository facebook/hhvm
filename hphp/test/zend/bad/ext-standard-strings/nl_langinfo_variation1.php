<?php

/* Prototype  : string nl_langinfo  ( int $item  )
 * Description: Query language and locale information
 * Source code: ext/standard/string.c
*/

echo "*** Testing nl_langinfo() : with unexpected inputs for 'item' argument ***\n";

$original = setlocale(LC_ALL, 'C');

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
$items =  array (
		  // integer values
/*1*/	  2147483647,
		  -2147483648,
		  -20,
		
	      // array values
/*4*/	  array(),
		  array(0),
		  array(1, 2),
		
		  // objects
/*7*/	  new sample(),
		
		  // resource
/*8*/	  $file_handle,
		);

//defining '$input' argument
$input = "Test string";

// loop through with each element of the $items array to test nl_langinfo() function
$count = 1;
foreach($items as $item) {
  echo "-- Iteration $count --\n";
  var_dump( nl_langinfo($item) );
  $count ++;
}

fclose($file_handle);  //closing the file handle
setlocale(LC_ALL, $original); 

?>
===DONE===