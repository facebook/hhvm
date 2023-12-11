<?hh
/* Prototype: bool is_string ( mixed $var );
 * Description: Finds whether the given variable is a string
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing is_string() with valid string values ***\n";
// different valid strings

/* string created using Heredoc (<<<) */
$heredoc_string = <<<EOT
This is string defined
using heredoc.
EOT;
/* heredoc string with only numerics */
$heredoc_numeric_string = <<<EOT
123456 3993
4849 string
EOT;
/* null heardoc string */
$heredoc_empty_string = <<<EOT
EOT;
$heredoc_null_string = <<<EOT
NULL
EOT;

$strings = vec[
  "",
  " ",
  '',
  ' ',
  "string",
  'string',
  "NULL",
  'null',
  "FALSE",
  'true',
  "\x0b",
  "\0",
  '\0',
  '\060',
  "\070",
  "0x55F",
  "055",
  "@#$#$%%$^^$%^%^$^&",
  $heredoc_string,
  $heredoc_numeric_string,
  $heredoc_empty_string,
  $heredoc_null_string
];
/* loop to check that is_string() recognizes different
   strings, expected output bool(true) */
$loop_counter = 1;
foreach ($strings as $string ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_string($string) );
}

echo "\n*** Testing is_string() on non string values ***\n";

// get a resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );

// other types in a array
$not_strings = varray [
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
  new stdClass,

  /* resources */
  $fp,
  $dfp,

  /* arrays */
  vec[],
  vec[0],
  vec[1],
  vec[NULL],
  vec[null],
  vec["string"],
  vec[true],
  vec[TRUE],
  vec[false],
  vec[FALSE],
  vec[1,2,3,4],
  dict[1 => "One", "two" => 2],
];
/* loop through the $not_strings to see working of
   is_string() on non string types, expected output bool(false) */
$loop_counter = 1;
foreach ($not_strings as $type ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_string($type) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
try { var_dump( is_string() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//arguments more than expected
try { var_dump( is_string("string", "test") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";

// close the resources used
fclose($fp);
closedir($dfp);
}
