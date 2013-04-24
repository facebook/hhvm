<?php
/* Prototype: bool is_null ( mixed $var );
 * Description: Finds whether the given variable is NULL 
 */

echo "*** Testing is_null() with valid null values ***\n";
// different valid  null vlaues 
$unset_array = array();
$unset_int = 10;
$unset_float = 10.5;
$unset_bool = true;
$unset_object = new stdclass;
$unset_resource = fopen(__FILE__, "r");
// unset them to make it null.
unset ($unset_array, $unset_int, $unset_float, $unset_bool, $unset_object, $unset_resource); 
$null_var1 = NULL;
$null_var2 = null;

$valid_nulls = array(
  NULL,
  null,
  @$null_var1,
  @$null_var2,
  @$unset_array,
  @$unset_int,
  @$unset_float,
  @$unset_bool,
  @$unset_object,
  @$unset_resource,
  @$undefined_var,
);
/* loop to check that is_null() recognizes different 
   null values, expected output: bool(true) */
$loop_counter = 1;
foreach ($valid_nulls as $null_val ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_null($null_val) );
}

echo "\n*** Testing is_bool() on non null values ***\n";

// get a resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );

// other types in a array 
$not_null_types = array (
/* integers */
  0,
  1,
  -1,
  -0,
  543915,
  -5322,
  0x0,
  0x1,
  0x55F,
  -0xCCF,
  0123,
  -0654,
  00,
  01,

  /* strings */
  "",
  '',
  "0",
  '0',
  "1",
  '1',
  'string',
  "string",
  "true",
  "false",
  "FALSE",
  "TRUE",
  'true',
  'false',
  'FALSE',
  'TRUE',
  "NULL",
  "null",

  /* floats */
  0.0,
  1.0,
  -1.0,
  10.0000000000000000005,
  .5e6,
  -.5E7,
  .5E+8,
  -.5e+90,
  1e5,
  -1e5,
  1E5,
  -1E7,

  /* objects */
  new stdclass,

  /* resources */
  $fp,
  $dfp,

  /* arrays */
  array(),
  array(0),
  array(1),
  array(NULL),
  array(null),
  array("string"),
  array(true),
  array(TRUE),
  array(false),
  array(FALSE),
  array(1,2,3,4),
  array(1 => "One", "two" => 2),
);
/* loop through the $not_null_types to see working of 
   is_null() on non null types, expected output: bool(false) */
$loop_counter = 1;
foreach ($not_null_types as $type ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_null($type) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
var_dump( is_null() );

//arguments more than expected 
var_dump( is_null(NULL, null) );
 
echo "Done\n";

// close the resources used
fclose($fp);
closedir($dfp);

?>