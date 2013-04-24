<?php
/* Prototype  : string quoted_printable_decode  ( string $str  )
 * Description: Convert a quoted-printable string to an 8 bit string
 * Source code: ext/standard/string.c
*/

/*
* Testing quoted_printable_decode() : with different unexpected values for format argument other than the strings
*/

echo "*** Testing quoted_printable_decode() : with unexpected values for 'str' argument ***\n";

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
/*1*/	      0,
		      1,
		      12345,
		      -2345,
		
		      // float data
/*5*/	      10.5,
		      -10.5,
		      10.1234567e10,
		      10.7654321E-10,
		      .5,
		
		      // array data
/*10*/	      array(),
		      array(0),
		      array(1),
		      array(1, 2),
		      array('color' => 'red', 'item' => 'pen'),
		
		      // null data
/*15*/	      NULL,
		      null,
		
		      // boolean data
/*17*/	      true,
		      false,
		      TRUE,
		      FALSE,
		
		      // empty data
/*21*/	      "",
		      '',
		
		      // object data
/*23*/	      new sample(),
		
		      // undefined data
/*24*/	      @$undefined_var,
		
		      // unset data
/*25*/	      @$unset_var,
		
		      // resource data
/*26*/	      $file_handle
);

// loop through each element of the array for 'str'

$count = 1;
foreach($values as $value) {
  echo "\n-- Iteration $count --\n";
  var_dump(bin2hex(quoted_printable_decode($value)));
  $count++;
};

// close the resource
fclose($file_handle);

?>
===DONE===