<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vfprintf() when different float formats and non-float values are passed to
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : float formats and non-float values ***\n";

// defining array of float formats
$formats = 
  '%f %+f %-f 
   %lf %Lf %4f %-4f
   %10.4f %-10.4f %04f %04.4f
   %\'#2f %\'2f %\'$2f %\'_2f
   %3$f %4$f %1$f %2$f';

// Arrays of non float values for the format defined in $format.
// Each sub array contains non float values which correspond to each format in $format
$args_array = vec[

  // array of int values
  vec[2, -2, +2,
        123456, 123456234, -12346789, +12346789,
        123200, +20000, -40000, 22212,
        12345780, 1211111, -12111111, -12345634,
        3, +4, 1,-2 ],

  // array of strings
  vec[" ", ' ', 'hello',
        '123hello', "123hello", '-123hello', '+123hello',
        "\12345678hello", "-\12345678hello", '0123456hello', 'h123456ello',
        "1234hello", "hello\0world", "NULL", "true",
        "3", "4", '1', '2'],

  // different arrays
  vec[ vec[0], vec[1, 2], vec[-1, -1],
         vec["123"], vec['123'], vec['-123'], vec["-123"],
         vec[true], vec[false], vec[TRUE], vec[FALSE],
         vec["123hello"], vec["1", "2"], vec['123hello'], dict[12=>"12twelve"],
         vec["3"], vec["4"], vec["1"], vec["2"] ],

  // array of boolean data
  vec[ true, TRUE, false,
         TRUE, 0, FALSE, 1,
         true, false, TRUE, FALSE,
         0, 1, 1, 0,
         1, TRUE, 0, FALSE],
  
];

/* creating dumping file */
$data_file = sys_get_temp_dir().'/'.'vfprintf_variation6.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

// looping to test vfprintf() with different float formats from the above $format array
// and with non-float values from the above $args_array array
$counter = 1;
foreach($args_array as $args) {
  fprintf($fp, "\n-- Iteration %d --\n",$counter);
  vfprintf($fp, $formats, $args);
  $counter++;
}

fclose($fp);
print_r(file_get_contents($data_file));
echo "\n";

unlink($data_file);

echo "===DONE===\n";
}
