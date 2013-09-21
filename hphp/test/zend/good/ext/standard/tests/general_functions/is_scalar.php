<?php
/* Prototype: bool is_scalar ( mixed $var );
 * Description: Finds whether a variable is a scalar (i.e integer, float, string or boolean)
 */

echo "*** Testing basic operations ***\n";
$scalar_variables = array(
  0,  // integers
  1,
  -45678,
  0x5FF,  // hexadecimal as integer
  0X566,
  -0xAAF, 
  -0XCCF,
  01234,  // octal as integer
  -0126,

  0.0,  // floats
  -1.0,
  1e5,
  -1e7,
  1.6E7,
  475.e-8,
  784.e+30,
  98.45E+40,
  .5E-40,

  "",  // strings
  '',
  " ",
  ' ',
  "string",
  'string', 
  "0",  // numeric as string  
  "40",
  "50.696",
  "0x534",
  "0X534",

  TRUE,  // boolean
  FALSE,
  true,
  false
);
/* loop through each valid scalar variables in $scalar_variables 
   and see the working of is_scalar(), expected output: bool(true)
*/
$loop_counter = 1;
foreach($scalar_variables as $scalar) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_scalar($scalar) );
}

echo "\n*** Testing possible variations ***\n";
// different scalar variables which are unset
$int_var = 10;
$float_var = 1e5;
$string_var = "string";
$boolean_var = true;
$object = new stdclass;
$array = array(10);
$resource = opendir('.');
unset($int_var, $float_var, $string_var, $boolean_var, $object, $array, $resource);

// resources 
$fp = fopen(__FILE__, "r");
$dfp = opendir(".");

$variation_array = array(
  NULL,
  null,

  array(),  // arrays 
  array(NULL),
  array(true),
  array(0),
  array(1,2,3,4),

  $fp,  // resources
  $dfp,

  new stdclass, // object

  @$int_var,  // scalars that are unset
  @$float_var,
  @$string_var,
  @$boolean_var,

  @$array,   // non scalars that are unset
  @$object,
  @$resource,

  @$undefined_var  // undefined variable
);  

/* loop through each element of $variation_array to see the 
   working of is_scalar on non-scalar values, expected output: bool(false)
*/
$loop_counter = 1;
foreach( $variation_array as $value ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_scalar($value) );
}

echo "\n*** Testing error conditions ***\n";
// Zero arguments
var_dump( is_scalar() );

// Arguments more than expected
var_dump( is_scalar( $scalar_variables[2], $scalar_variables[2]) );
var_dump( is_scalar( new stdclass, new stdclass) );

echo "Done\n";  

// close the resources used
fclose($fp);
closedir($dfp);

?>