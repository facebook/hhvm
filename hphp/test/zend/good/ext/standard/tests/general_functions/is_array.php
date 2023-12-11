<?hh
/* Prototype: bool is_array ( mixed $var );
 * Description: Finds whether the given variable is an array
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing is_array() on different type of arrays ***\n";
/* different types of arrays */
$arrays = vec[
  vec[],
  vec[NULL],
  vec[null],
  vec[true],
  vec[""],
  vec[''],
  vec[vec[], vec[]],
  vec[vec[1, 2], vec['a', 'b']],
  dict[1 => 'One'],
  dict["test" => "is_array"],
  vec[0],
  vec[-1],
  vec[10.5, 5.6],
  vec["string", "test"],
  vec['string', 'test']
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
  new stdClass,

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
