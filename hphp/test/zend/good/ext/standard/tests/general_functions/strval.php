<?hh
/* Prototype: string strval ( mixed $var );
 * Description: Returns the string value of var
 */

// non_scalar values, objects, arrays, resources and boolean
class foo
{
  function __toString() :mixed{
    return "Object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing str_val() with scalar values***\n";
$heredoc_string = <<<EOD
This is a multiline heredoc
string. Numeric = 1232455.
EOD;
/* heredoc string with only numeric values */
$heredoc_numeric_string = <<<EOD
12345
2345
EOD;
/* null heredoc string */
$heredoc_empty_string = <<<EOD
EOD;
/* heredoc string with NULL */
$heredoc_NULL_string = <<<EOD
NULL
EOD;

// different valid  scalar vlaues
$scalars = vec[
  /* integers */
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

  /* floats */
  -0x80000000, // max negative integer as hexadecimal
  0x7fffffff,  // max postive integer as hexadecimal
  0x7FFFFFFF,  // max postive integer as hexadecimal
  0123,        // integer as octal
  01,       // should be quivalent to octal 1
  -020000000000, // max negative integer as octal
  017777777777,  // max positive integer as octal
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
  1e-5,
  .5e+7,
  .6e-19,
  .05E+44,
  .0034E-30,

  /* booleans */
  true,
  TRUE,
  FALSE,
  false,

  /* strings */
  "",
  '',
  " ",
  ' ',
  '0',
  "0",
  "testing",
  "0x564",
  "0123",
  "new\n",
  'new\n',
  "@#$$%^&&*()",
  "        ",
  "null",
  'null',
  'true',
  "true",
  /*"\0123",
  "\0x12FF",*/
  $heredoc_string,
  $heredoc_numeric_string,
  $heredoc_empty_string
];
/* loop to check that strval() recognizes different
   scalar values and retuns the string conversion of same */
$loop_counter = 1;
foreach ($scalars as $scalar ) {
   echo "-- Iteration $loop_counter --\n"; $loop_counter++;
   var_dump( strval($scalar) );
}

echo "\n*** Testing strval() with non_scalar values ***\n";
// get a resource type variable
$fp = fopen(__FILE__, "r");
$dfp = opendir( dirname(__FILE__) );


$not_scalars = varray [
  new foo, //object
  $fp,  // resource
  $dfp,
  NULL,  // nulls
  null,
];
/* loop through the $not_scalars to see working of
   strval() on objects, arrays, boolean and others */
$loop_counter = 1;
foreach ($not_scalars as $value ) {
   echo "-- Iteration $loop_counter --\n"; $loop_counter++;
   var_dump( strval($value) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
try { var_dump( strval() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//arguments more than expected
try { var_dump( strval( $scalars[0], $scalars[1]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";

// close the resources used
fclose($fp);
closedir($dfp);
}
