<?php
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vfprintf() when different unexpected format strings are passed to
 * the '$format' argument of the function
*/

echo "*** Testing vfprintf() : with unexpected values for format argument ***\n";

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

/* creating dumping file */
$data_file = dirname(__FILE__) . '/vfprintf_variation20.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

fprintf($fp, "\n*** Testing vprintf() with with unexpected values for format argument ***\n");

$counter = 1;
foreach( $values as $value ) {
  fprintf( $fp, "\n-- Iteration %d --\n",$counter);
  vfprintf($fp, $value, $args);
  $counter++;
}

fclose($fp);
print_r(file_get_contents($data_file));
echo "\n";

unlink($data_file);

?>
===DONE===