<?php
/* Prototype  : int printf  ( string $format  [, mixed $args  [, mixed $...  ]] )
 * Description: Produces output according to format .
 * Source code: ext/standard/formatted_print.c
 */

error_reporting(E_ALL & ~E_NOTICE);

echo "*** Testing printf() : with different types of values passed for arg1 argument ***\n";

// initialing required variables
$format = '%s';
$arg2 = 'third argument';

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
/*1*/     0,
	      1,
	      12345,
	      -2345,
	
	      // float data
/*5*/     10.5,
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
	
	      // string data
/*23*/    "string",
	      'string',
	
	      // object data
/*25*/    new sample(),
	
	      // undefined data
/*26*/    @$undefined_var,
	
	      // unset data
/*27*/    @$unset_var,
	
	      // resource data
/*28*/    $file_handle
);

// loop through each element of the array for arg1

$count = 1;
foreach($values as $value) {
  echo "\n-- Iteration $count --\n";
  
  // with two arguments
  $result = printf($format, $value);
  echo "\n";
  var_dump($result);

  // with three arguments
  $result = printf($format, $value, $arg2);
  echo "\n";
  var_dump($result);
 
  $count++;   
};

// closing the resource
fclose($file_handle);

?>
===DONE===