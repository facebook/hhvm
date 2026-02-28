<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : string formats with string values ***\n";

// defining different heredoc strings
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

// array of strings used to test the function
$string_values = vec[
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

// array of string formats
$string_formats = vec[ 
  "%s", "%hs", "%ls",
  "%Ls"," %s", "%s ",
  "\t%s", "\n%s", "%4s",
  "%30s", "%[a-zA-Z0-9]", "%*s"
];

$count = 1;
foreach($string_values as $string_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($string_formats as $format) {
    var_dump( sprintf($format, $string_value) );
  }
  $count++;
};

echo "Done";
}
