<?hh
/* Prototype: bool is_null ( mixed $var );
 * Description: Finds whether the given variable is NULL 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing is_null() with valid null values ***\n";
// different valid  null vlaues 
$unset_array = varray[];
$unset_int = 10;
$unset_float = 10.5;
$unset_bool = true;
$unset_object = new stdclass;
$unset_resource = fopen(__FILE__, "r");
// unset them to make it null.
unset ($unset_array, $unset_int, $unset_float, $unset_bool, $unset_object, $unset_resource); 
$null_var1 = NULL;
$null_var2 = null;

$valid_nulls = varray[
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
];
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
$not_null_types = varray [
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
];
/* loop through the $not_null_types to see working of 
   is_null() on non null types, expected output: bool(false) */
$loop_counter = 1;
foreach ($not_null_types as $type ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_null($type) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
try { var_dump( is_null() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//arguments more than expected 
try { var_dump( is_null(NULL, null) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
 
echo "Done\n";

// close the resources used
fclose($fp);
closedir($dfp);
}
