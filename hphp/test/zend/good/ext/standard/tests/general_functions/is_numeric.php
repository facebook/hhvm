<?php
/* Prototype: bool is_numeric ( mixed $var );
 * Description: Finds whether a variable is a number or a numeric string  
 */

echo "*** Testing is_numeric() with valid numeric values ***\n";
// different valid numeric  vlaues 
$numerics = array(
  0,
  1,
  -1,
  -0,
  +0,
  0.0,
  -0.0,
  +0.0,
  1.0,
  -1.0,
  +1.0,
  .5,
  -.5,
  +.5,
  -.5e-2,
  .5e-2,
  +.5e-2,
  +.5E+2,
  0.70000000,
  +0.70000000,
  -0.70000000,
  1234567890123456,
  -1234567890123456,
  984847472827282718178,
  -984847472827282718178,
  123.56e30,
  123.56E30,
  426.45e-30,
  5657.3E-40,
  3486.36e+40,
  3486.36E+90,
  -3486.36E+10,
  -3486.36e+80,
  -426.45e-50,
  -426.45E-99,
  1e2,
  -1e2,
  -1e-2,
  +1e2,
  +1e+2,
  +1e-2,
  +1e+2,
  2245555555555555.444,
  1.444444444444444444,
  0xff,	// hexa decimal numbers
  0xFF,
  //0x1111111111111111111111,
  -0x1111111,
  +0x6698319,
  01000000000000000000000, 
  0123,
  0345900,
  -0200001,  
  -0200001.7,  
  0200001.7,  
  +0200001,  
  +0200001.7,  
  +0200001.7,  
  2.00000000000000000000001, // a float value with more precision points
  "1",	// numeric in the form of string
  "-1",
  "1e2",
  " 1",
  "2974394749328742328432",
  "-1e-2",
  '1',
  '-1',
  '1e2',
  ' 1',
  '2974394749328742328432',
  '-1e-2',
  "0xff",
  '0xff',
  "0123",
  '0123',
  "-0123",
  "+0123",
  '-0123',
  '+0123'
);
/* loop to check that is_numeric() recognizes different 
   numeric values, expected output: bool(true) */
$loop_counter = 1;
foreach ($numerics as $num ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_numeric($num) );
}

echo "\n*** Testing is_numeric() on non numeric types ***\n";

// get a resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );

// unset variable
$unset_var = 10.5;
unset ($unset_var);

// other types in a array 
$not_numerics = array(
  "-0x80001", 
  "+0x80001", 
  "-0x80001.5",
  "0x80001.5", 
  new stdclass, // object
  $fp,  // resource
  $dfp,
  array(),
  array("string"),
  "",
  "1 ",
  "- 1",
  "1.2.4",
  "1e7.6",
  "3FF",
  "20 test",
  "3.6test",
  "1,000",
  "NULL", 
  "true",
  true,
  NULL,
  null,
  TRUE,
  FALSE,
  false,
  @$unset_var, // unset variable
  @$undefined_var
);
/* loop through the $not_numerics to see working of 
   is_numeric() on non numeric values, expected output: bool(false) */
$loop_counter = 1;
foreach ($not_numerics as $type ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_numeric($type) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
var_dump( is_numeric() );

//arguments more than expected 
var_dump( is_numeric("10", "20") );
 
echo "Done\n";

// close the resources used
fclose($fp);
closedir($dfp);

?>