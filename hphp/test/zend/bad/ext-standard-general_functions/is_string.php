<?php
/* Prototype: bool is_string ( mixed $var );
 * Description: Finds whether the given variable is a string
 */

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

$strings = array(
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
);
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

// unset vars
$unset_string1 = "string";
$unset_string2 = 'string';
$unset_heredoc = <<<EOT
this is heredoc string
EOT;
// unset the vars 
unset($unset_string1, $unset_string2, $unset_heredoc);

// other types in a array 
$not_strings = array (
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

  /* undefined and unset vars */
  @$unset_string1,
  @$unset_string2,
  @$unset_heredoc,
  @$undefined_var 
);
/* loop through the $not_strings to see working of 
   is_string() on non string types, expected output bool(false) */
$loop_counter = 1;
foreach ($not_strings as $type ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_string($type) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
var_dump( is_string() );

//arguments more than expected 
var_dump( is_string("string", "test") );
 
echo "Done\n";

// close the resources used
fclose($fp);
closedir($dfp);

?>