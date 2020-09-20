<?hh
/* Prototype: bool is_bool ( mixed $var );
 * Description: Finds whether the given variable is a boolean  
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing is_bool() with valid boolean values ***\n";
// different valid  boolean vlaues 
$valid_bools = varray[
  TRUE,
  FALSE,
  true,
  false,
];
/* loop to check that is_bool() recognizes different 
   bool values, expected output: bool(true) */
$loop_counter = 1;
foreach ($valid_bools as $bool_val ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++; 
  var_dump( is_bool($bool_val) );
}

echo "\n*** Testing is_bool() on non boolean values ***\n";

// get a resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );

// unset variable
$unset_bool1 = true;
$unset_bool2 = false;
$unset_var = 0;
unset ($unset_bool1);
unset ($unset_bool2);
unset ($unset_var);

// other types in a array 
$not_bool_types = varray [
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

  /* nulls */
  null,
  NULL,
  
  /* arrays */
  varray[],
  varray[0],
  varray[1],
  varray[NULL],
  varray[null],
  varray["string"],
  varray[true],
  varray[TRUE],
  varray[false],
  varray[FALSE],
  varray[1,2,3,4],
  darray[1 => "One", "two" => 2],

  /* unset bool vars and undefined var */
  @$unset_bool1, 
  @$unset_bool2, 
  @$unset_var, 
  @$undefined_var
];
/* loop through the $not_bool_types to see working of 
   is_bool() on non bull types, expected output: bool(false) */
$loop_counter = 1;
foreach ($not_bool_types as $type ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++; 
  var_dump( is_bool($type) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
try { var_dump( is_bool() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//arguments more than expected 
try { var_dump( is_bool(TRUE, FALSE) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
 
echo "Done\n";

// close resources
fclose($fp);
closedir($dfp);
}
