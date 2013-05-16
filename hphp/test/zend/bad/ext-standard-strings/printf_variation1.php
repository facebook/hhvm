<?php
/* Prototype  : int printf  ( string $format  [, mixed $args  [, mixed $...  ]] )
 * Description: Produces output according to format .
 * Source code: ext/standard/formatted_print.c
 */

/*
* Testing printf() : with different unexpected values for format argument other than the strings
*/

echo "*** Testing printf() : with unexpected values for format argument ***\n";

// initialing required variables
$arg1 = "second arg";
$arg2 = "third arg";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// declaring class
class sample
{
  public function __toString() {
    return "Object";
  }
}

// creating a file resource
$file_handle = fopen(__FILE__, 'r');

//array of values to iterate over
$values = array(

		  // int data
/*1*/	  0,
	      1,
	      12345,
	      -2345,
	
	      // float data
/*5*/      10.5,
	      -10.5,
	      10.1234567e10,
	      10.7654321E-10,
	      .5,
	
	      // array data
/*10*/    array(),
	      array(0),
	      array(1),
	      array(1, 2),
	      array('color' => 'red', 'item' => 'pen'),
	
	      // null data
/*15*/    NULL,
	      null,
	
	      // boolean data
/*17*/    true,
	      false,
	      TRUE,
	      FALSE,
	
	      // empty data
/*21*/    "",
	      '',
	
	      // object data
/*23*/    new sample(),
	
	      // undefined data
/*24*/    @$undefined_var,
	
	      // unset data
/*25*/    @$unset_var,
	
	      // resource data
/*26*/    $file_handle
);

// loop through each element of the array for format

$count = 1;
foreach($values as $value) {
  echo "\n-- Iteration $count --\n";
  
  // with default argument
  $result = printf($value);
  echo "\n";
  var_dump($result);
  
  // with two arguments
  $result = printf($value, $arg1);
  echo "\n";
  var_dump($result);

  // with three arguments
  $result = printf($value, $arg1, $arg2);
  echo "\n";
  var_dump($result);

  $count++;
};

// close the resource
fclose($file_handle);

?>
===DONE===