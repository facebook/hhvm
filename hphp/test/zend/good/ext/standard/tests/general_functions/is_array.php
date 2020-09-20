<?hh
/* Prototype: bool is_array ( mixed $var );
 * Description: Finds whether the given variable is an array
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing is_array() on different type of arrays ***\n";
/* different types of arrays */
$arrays = varray[
  varray[],
  varray[NULL],
  varray[null],
  varray[true],
  varray[""],
  varray[''],
  varray[varray[], varray[]],
  varray[varray[1, 2], varray['a', 'b']],
  darray[1 => 'One'],
  darray["test" => "is_array"],
  varray[0],
  varray[-1],
  varray[10.5, 5.6],
  varray["string", "test"],
  varray['string', 'test']
];
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
$unset_array = varray[10];
unset($unset_array);

// other types in a array
$varient_arrays = varray [
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
];
/* loop through the $varient_array to see working of
   is_array() on non array types, expected output bool(false) */
$loop_counter = 1;
foreach ($varient_arrays as $type ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_array ($type) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
try { var_dump( is_array() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//arguments more than expected
try { var_dump( is_array ($fp, $fp) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
/* close resources */
fclose($fp);
closedir($dfp);
}
