<?hh
/* Prototype: bool is_float ( mixed $var );
 * Description: Finds whether the given variable is a float
 */
class foo {
  public $array = vec[10.5];
}

<<__EntryPoint>>
function main(): void {
  echo "*** Testing is_float(), is_double() and is_real() with float values***\n";
  // different valid  float vlaues
  $floats = vec[
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
  ];
  /* loop to check that is_float(), is_double() & is_real() recognizes
     different float values, expected: bool(true)  */
  $loop_counter = 1;
  foreach ($floats as $float) {
    echo "-- Iteration $loop_counter --\n";
    $loop_counter++;
    var_dump(is_float($float));
    var_dump(is_double($float));
    var_dump(is_real($float));
  }

  echo "\n*** Testing is_float(), is_double() & is_real() with non float values ***\n";
  // get a resource type variable
  $fp = fopen(__FILE__, "r");
  $dfp = opendir(dirname(__FILE__));


  // non_scalar values, objects, arrays, resources and boolean
  $object = new foo();

  $not_floats = vec[
    new foo, //object
    $object,

    $fp,  // resource
    $dfp,

    vec[],  // arrays
    vec[NULL],
    vec[0.5e10],
    vec[1,2,3,4],
    vec["string"],

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

  ];
  /* loop through the $not_floats to see working of
     is_float(), is_double() & is_real() on objects,
      arrays, boolean and others */
  $loop_counter = 1;
  foreach ($not_floats as $value) {
    echo "--Iteration $loop_counter--\n";
    $loop_counter++;
    var_dump(is_float($value));
    var_dump(is_double($value));
    var_dump(is_real($value));
  }

  echo "\n*** Testing error conditions ***\n";
  //Zero argument
  try {
    var_dump(is_float());
  } catch (Exception $e) {
    echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
  }
  try {
    var_dump(is_double());
  } catch (Exception $e) {
    echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
  }
  try {
    var_dump(is_real());
  } catch (Exception $e) {
    echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
  }

  //arguments more than expected
  try {
    var_dump(is_float($floats[0], $floats[1]));
  } catch (Exception $e) {
    echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
  }
  try {
    var_dump(is_double($floats[0], $floats[1]));
  } catch (Exception $e) {
    echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
  }
  try {
    var_dump(is_real($floats[0], $floats[1]));
  } catch (Exception $e) {
    echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
  }

  echo "Done\n";
}
