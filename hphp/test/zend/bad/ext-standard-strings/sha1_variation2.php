<?php

/* Prototype: string sha1  ( string $str  [, bool $raw_output  ] )
 * Description: Calculate the sha1 hash of a string
 */

echo "*** Testing sha1() : unexpected values for 'raw' ***\n";

$string = "Hello World";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//defining class for object variable
class MyClass
{
  public function __toString()
  {
    return "object";
  }
}

//resource variable
$fp = fopen(__FILE__, 'r');

//different values for 'str' argument
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
		  10.1234567E-10,
		  .5,
		
		  // array data
/*10*/	  array(),
		  array(0),
		  array(1),
		  array(1, 2),
		  array('color' => 'red', 'item' => 'pen'),
		
		  // null data
/*15*/	  NULL,
		  null,
		
  		  // string data
/*17*/	  "ABC",
  		  'abc',
  		  "0abc",
  		  "123abc",
		
		  // empty data
/*21*/	  "",
		  '',
		
		  // object data
/*23*/	  new MyClass(),
		
		  // undefined data
/*24*/	  @$undefined_var,
		
		  // unset data
/*25*/	  @$unset_var,
		
		  //resource data
/*26*/	  $fp
);

// loop through each element of $values for 'raw' argument
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  // use bin2hex to catch those cases were raw is true
  var_dump( bin2hex(sha1($string, $values[$count])) );
}

//closing resource
fclose($fp);

?>
===DONE===