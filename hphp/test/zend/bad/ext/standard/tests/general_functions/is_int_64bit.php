<?php
/* Prototype: bool is_int ( mixed $var );
 * Description: Finds whether the given variable is an integer  
 */

echo "*** Testing is_int(), is_integer() & is_long()  with valid integer values ***\n";
// different valid  integer vlaues 
$valid_ints = array(
  0,
  1,
  -1,
  -2147483648, // max negative integer value
  -2147483647, 
  2147483647,  // max positive integer value
  2147483640,
  0x123B,      // integer as hexadecimal
  0x12ab,
  0Xfff,
  0XFA,
  -0x80000000, // max negative integer as hexadecimal
  0x7fffffff,  // max postive integer as hexadecimal
  0x7FFFFFFF,  // max postive integer as hexadecimal
  0123,        // integer as octal
  01912,       // should be quivalent to octal 1
  -020000000000, // max negative integer as octal
  017777777777,  // max positive integer as octal
);
/* loop to check that is_int() recognizes different 
   integer values, expected output: bool(true) */
$loop_counter = 1;
foreach ($valid_ints as $int_val ) {
   echo "--Iteration $loop_counter--\n"; $loop_counter++;   
   var_dump( is_int($int_val) );
   var_dump( is_integer($int_val) );
   var_dump( is_long($int_val) );
}

echo "\n*** Testing is_int(), is_integer() & is_long() with  non integer values ***\n";

// resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );
// unset variable

$unset_var = 10;
unset ($unset_var);

// other types in a array 
$not_int_types = array (
  /* float values */
  -2147483649, // float value
  2147483648,  // float value
  -0x80000001, // float value, beyond max negative int
  0x800000001, // float value, beyond max positive int
  020000000001, // float value, beyond max positive int 
  -020000000001, // float value, beyond max negative int 
  0.0,  
  -0.1,
  1.0,
  1e5,
  -1e6,
  1E8,
  -1E9,
  10.0000000000000000005,
  10.5e+5,
 
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
  
  /* strings */
  "",
  '',
  "0",
  '0',
  "1",
  '1',
  "\x01",
  '\x01',
  "\01",
  '\01',
  'string',
  "string",
  "true",
  "FALSE",
  'false',
  'TRUE',
  "NULL",
  'null',

  /* booleans */
  true,
  false,
  TRUE,
  FALSE,

  /* undefined and unset vars */
  @$unset_var, 
  @$undefined_var
);
/* loop through the $not_int_types to see working of 
   is_int() on non integer types, expected output: bool(false) */
$loop_counter = 1;
foreach ($not_int_types as $type ) {
   echo "--Iteration $loop_counter--\n"; $loop_counter++;   
   var_dump( is_int($type) );
   var_dump( is_integer($type) );
   var_dump( is_long($type) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
var_dump( is_int() );
var_dump( is_integer() );
var_dump( is_long() );

//arguments more than expected 
var_dump( is_int(TRUE, FALSE) );
var_dump( is_integer(TRUE, FALSE) );
var_dump( is_long(TRUE, FALSE) );
 
echo "Done\n";
?>
