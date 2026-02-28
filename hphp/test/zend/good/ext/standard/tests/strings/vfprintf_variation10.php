<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vfprintf() when different char formats and non-char values are passed to 
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : char formats and non-char values ***\n";

// defining an array of various char formats
$formats = 
  '%c %+c %-c 
   %lc %Lc %4c %-4c
   %10.4c %-10.4c %04c %04.4c
   %\'10c %\'10c %\'$10c %\'_10c
   %3$c %4$c %1$c %2$c';

// Arrays of non char values for the format defined in $format. 
// Each sub array contains non char values which correspond to each format in $format
$args_array = vec[

  // array of float values
  vec[65.8, -65.8, +66.8,
        93.2, -93.2, 126.8, -126.49,
        35.44, -35.68, 32.99, -32.00,
        -61.51, 61.51, 50.49, -54.50,
        83.33, +84.44, 81.11, 82.22],

  // array of int values
  vec[65, -65, +66,
        169, -169, 126, -126,
        35, -35, 32, -32,
        -61, 61, 50, -54,
        83, +84, 81, 82],

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
$data_file = sys_get_temp_dir().'/'.'vfprintf_variation10.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

// looping to test vfprintf() with different char formats from the above $format array
// and with non-char values from the above $args_array array
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
