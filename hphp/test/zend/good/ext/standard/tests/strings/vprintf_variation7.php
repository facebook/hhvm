<?hh
/* Prototype  : string vprintf(string format, array args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vprintf() when different string formats and string values are passed to
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : string formats with strings ***\n";


// defining different heredoc strings
$heredoc_string = <<<EOT
This is string defined
using heredoc.
EOT;

/* heredoc string with only numerics */
$heredoc_numeric_string = <<<EOT
123456 3993
4849 string
EOT;

/* empty heardoc string */
$heredoc_empty_string = <<<EOT
EOT;

// defining array of string formats
$formats = vec[
  "%s",
  "%+s %-s %S",
  "%ls %Ls, %4s %-4s",
  "%10.4s %-10.4s %04s %04.4s",
  "%'#2s %'2s %'$2s %'_2s",
  "%% %%s %10 s%",
  '%3$s %4$s %1$s %2$s'
];

// Arrays of string values for the format defined in $format.
// Each sub array contains string values which correspond to each format string in $format
$args_array = vec[
  vec[" "],
  vec["hello\0world", "hello\0", "\0hello"],
  vec["@#$%&*", "@#$%&*", "\x55F", "\001"],
  vec["sunday", 'monday', "tuesday", 'wednesday'],
  vec[$heredoc_string, "abcdef", $heredoc_numeric_string, $heredoc_empty_string],
  vec["one", "two", 'three', 'four'],
  vec["three", 'four', 'one', "two"]

];

// looping to test vprintf() with different string formats from the above $format array
// and with string from the above $args_array array
$counter = 1;
foreach($formats as $format) {
  echo "\n-- Iteration $counter --\n";   
  $result = vprintf($format, $args_array[$counter-1]);
  echo "\n";
  var_dump($result);
  $counter++;
}

echo "===DONE===\n";
}
