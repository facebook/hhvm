<?php
ini_set('precision', 14);

/* Prototype: string gettype ( mixed $var );
   Description: Returns the type of the PHP variable var

   Prototype: bool settype ( mixed &$var, string $type );
   Description: Set the type of variable var to type 
*/

/* Test the basic functionalities of settype() & gettype() functions.
   Use the gettype() to get the type of regular data and use settype() 
   to change its type to other types */

/* function to handle catchable errors */
function foo($errno, $errstr, $errfile, $errline) {
//	var_dump($errstr);
   // print error no and error string
   echo "$errno: $errstr\n";
}
//set the error handler, this is required as
// settype() would fail with catachable fatal error 
set_error_handler("foo"); 

echo "**** Testing gettype() and settype() functions ****\n";

$fp = fopen(__FILE__, "r");
$dfp = opendir( dirname(__FILE__) );

$var1 = "another string";
$var2 = array(2,3,4);

class point
{
  var $x;
  var $y;

  function point($x, $y) {
     $this->x = $x;
     $this->y = $y;
  }

  function __toString() {
     return "Object";
  }
}

$unset_var = 10;
unset($unset_var);

$values = array(
  array(1,2,3),
  $var1,
  $var2,
  1,
  -20,
  2.54,
  -2.54,
  NULL,
  false,
  "some string",
  'string',
  $fp,
  $dfp,
  new point(10,20)
);

$types = array(
  "null",
  "integer",
  "int",
  "float",
  "double",
  "boolean",
  "bool",
  "resource",
  "array",
  "object",
  "string"
);

echo "\n*** Testing gettype(): basic operations ***\n";
foreach ($values as $value) {
  var_dump( gettype($value) );
}

echo "\n*** Testing settype(): basic operations ***\n";
foreach ($types as $type) {
  echo "\n-- Setting type of data to $type --\n"; 
  $loop_count = 1;
  foreach ($values as $var) {
     echo "-- Iteration $loop_count --\n"; $loop_count ++;
     // set to new type
     var_dump( settype($var, $type) );
    
     // dump the var 
     var_dump( $var );
  
     // check the new type 
     var_dump( gettype($var) );  
  }
}

echo "Done\n";
?>