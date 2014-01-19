<?php
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vfprintf() when different unexpected values are passed to
 * the '$args' arguments of the function
*/

echo "*** Testing vfprintf() : with unexpected values for args argument ***\n";

// initialising the required variables
$format = '%s';

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
		
		  // null data
/*10*/	  NULL,
		  null,
		
		  // boolean data
/*12*/	  true,
		  false,
		  TRUE,
		  FALSE,
		
		  // empty data
/*16*/	  "",
		  '',
		
		  // string data
/*18*/	  "string",
		  'string',
		
		  // object data
/*20*/	  new sample(),
		
		  // undefined data
/*21*/	  @$undefined_var,
		
		  // unset data
/*22*/	  @$unset_var,
		
		  // resource data
/*23*/	  $file_handle
);

/* creating dumping file */
$data_file = dirname(__FILE__) . '/vfprintf_variation21.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

fprintf($fp, "\n*** Testing vprintf() with unexpected values for args argument ***\n");

$counter = 1;
foreach( $values as $value ) {
  fprintf($fp, "\n-- Iteration %d --\n",$counter);
  vfprintf($fp, $format, $value);
  $counter++;
}

fclose($fp);
print_r(file_get_contents($data_file));
echo "\n";

unlink($data_file);


?>
===DONE===