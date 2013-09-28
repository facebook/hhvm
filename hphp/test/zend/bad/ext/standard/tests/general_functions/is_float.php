<?php
/* Prototype: bool is_float ( mixed $var );
 * Description: Finds whether the given variable is a float 
 */ 

echo "*** Testing is_float(), is_double() and is_real() with float values***\n";
// different valid  float vlaues 
$floats = array(
  -2147483649, // float value
  2147483648,  // float value
  -0x80000001, // float value, beyond max negative int
  0x800000001, // float value, beyond max positive int
  020000000001, // float value, beyond max positive int
  -020000000001, // float value, beyond max negative int
  0.0,
  -0.1,
  10.0000000000000000005,
  10.5e+5,
  1e5,
  -1e5,
  1e-5,
  -1e-5,
  1e+5,
  -1e+5,
  1E5,
  -1E5,
  1E+5,
  -1E+5,
  1E-5,
  -1E-5,
  .5e+7,
  -.5e+7,
  .6e-19,
  -.6e-19,
  .05E+44,
  -.05E+44,
  .0034E-30,
  -.0034E-30
);
/* loop to check that is_float(), is_double() & is_real() recognizes
   different float values, expected: bool(true)  */
$loop_counter = 1;
foreach ($floats as $float ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_float($float) );
  var_dump( is_double($float) );
  var_dump( is_real($float) );
}

echo "\n*** Testing is_float(), is_double() & is_real() with non float values ***\n";
// get a resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );

// unset variable
$unset_var = 10;
unset ($unset_var);

// non_scalar values, objects, arrays, resources and boolean 
class foo
{
  var $array = array(10.5);
};
$object = new foo();

$not_floats = array (
  new foo, //object
  $object,  

  $fp,  // resource
  $dfp,

  array(),  // arrays
  array(NULL),
  array(0.5e10),
  array(1,2,3,4),
  array("string"),

  NULL,  // nulls
  null,

  true,  // boolean
  TRUE,
  false,
  FALSE,
  
  "",  // strings
  '',
  "0",
  '0',
  "0.0",
  '0.0',
  '0.5',
  "-0.5",
  "1e5",
  '1e5',
  '1.5e6_string',
  "1.5e6_string",
 
  1,  // integers, hex and octal
  -1,
  0,
  12345,
  0xFF55,
  -0x673,
  0123,
  -0123,
   
  @$unset_var,  // unset variable
  @$undefined_var
);
/* loop through the $not_floats to see working of 
   is_float(), is_double() & is_real() on objects,
    arrays, boolean and others */
$loop_counter = 1;
foreach ($not_floats as $value ) {
  echo "--Iteration $loop_counter--\n"; $loop_counter++;
  var_dump( is_float($value) );
  var_dump( is_double($value) );
  var_dump( is_real($value) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
var_dump( is_float() );
var_dump( is_double() );
var_dump( is_real() );

//arguments more than expected 
var_dump( is_float( $floats[0], $floats[1]) );
var_dump( is_double( $floats[0], $floats[1]) );
var_dump( is_real( $floats[0], $floats[1]) );
 
echo "Done\n";

// close the resources used 
fclose($fp);
closedir($dfp);

?>