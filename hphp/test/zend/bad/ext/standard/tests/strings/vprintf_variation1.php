<?php
/* Prototype  : string vprintf(string $format, array $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vprintf() when different unexpected format strings are passed to
 * the '$format' argument of the function
*/

echo "*** Testing vprintf() : with unexpected values for format argument ***\n";

// initialising the required variables
$args = array(1, 2);

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// declaring a class
class sample
{
  public function __toString() {
  return "object";
  }
}

// Defining resource
$file_handle = fopen(__FILE__, 'r');


//array of values to iterate over
$values = array(

		  // int data
/*1*/	  0,
		  1,
		  12345,
		  -2345,
		
		  // float data
/*5*/	  10.5,
		  -10.5,
		  10.1234567e10,
		  10.7654321E-10,
		  .5,
		
		  // array data
/*10*/	  array(),
		  array(0),
		  array(1),
		  array(1,2),
		  array('color' => 'red', 'item' => 'pen'),
		
		  // null data
/*15*/	  NULL,
		  null,
		
		  // boolean data
/*17*/	  true,
		  false,
		  TRUE,
		  FALSE,
		
		  // empty data
/*21*/	  "",
		  '',
		
		  // object data
/*23*/	  new sample(),
		
		  // undefined data
/*24*/	  @$undefined_var,
		
		  // unset data
/*25*/	  @$unset_var,
		 
		  // resource data
/*26*/	  $file_handle
);

// loop through each element of the array for format

$counter = 1;
foreach($values as $value) {
  echo "\n -- Iteration $counter --\n";
  $result = vprintf($value,$args);
  echo "\n";
  var_dump($result); 
  $counter++;
    
};

// closing the resource
fclose($file_handle);

?>
===DONE===