<?php
/* Prototype: bool is_array ( mixed $var );
 * Description: Finds whether the given variable is an array
 */

echo "*** Testing is_array() on different type of arrays ***\n";
/* different types of arrays */
$arrays = array(
  array(),
  array(NULL),
  array(null),
  array(true),
  array(""),
  array(''),
  array(array(), array()),
  array(array(1, 2), array('a', 'b')),
  array(1 => 'One'),
  array("test" => "is_array"),
  array(0),
  array(-1),
  array(10.5, 5.6),
  array("string", "test"),
  array('string', 'test')
);
/* loop to check that is_array() recognizes different 
   type of arrays, expected output bool(true) */
$loop_counter = 1;
foreach ($arrays as $var_array ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_array ($var_array) );
}

echo "\n*** Testing is_array() on non array types ***\n";

// get a resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );

// unset variables 
$unset_array = array(10);
unset($unset_array);

// other types in a array 
$varient_arrays = array (
  /* integers */
  543915, 
  -5322,
  0x55F,
  -0xCCF,
  123,
  -0654,

  /* strings */
  "",  
  '',
  "0",
  '0',
  'string',
  "string",

  /* floats */
  10.0000000000000000005,
  .5e6,
  -.5E7,
  .5E+8,
  -.5e+90,
  1e5,
  
  /* objects */
  new stdclass, 
  
  /* resources */
  $fp, 
  $dfp, 

  /* nulls */
  null,  
  NULL,

  /* boolean */
  true, 
  TRUE,
  FALSE,
  false,

  /* unset/undefined arrays  */
  @$unset_array,
  @$undefined_array
);
/* loop through the $varient_array to see working of 
   is_array() on non array types, expected output bool(false) */
$loop_counter = 1;
foreach ($varient_arrays as $type ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_array ($type) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
var_dump( is_array() );

//arguments more than expected 
var_dump( is_array ($fp, $fp) );
 
echo "Done\n";
/* close resources */
fclose($fp);
closedir($dfp);
?>