<?hh
/* Prototype  : string vsprintf(string format, array args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vsprintf() when different unsigned formats and signed values and other types of values
 * are passed to the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vsprintf() : unsigned formats and signed & other types of values ***\n";

// defining array of unsigned formats
$formats = 
  '%u %+u %-u 
   %lu %Lu %4u %-4u
   %10.4u %-10.4u %.4u 
   %\'#2u %\'2u %\'$2u %\'_2u
   %3$u %4$u %1$u %2$u';

// Arrays of signed and other type of values for the format defined in $format.
// Each sub array contains signed values which correspond to each format in $format
$args_array = vec[

  // array of float values
  vec[+2.2, +.2, +10.2,
        +123456.234, +123456.234, +1234.6789,
        +2e10, +2e12, +22e+12,
        +12345.780, +12.000000011111, -12.00000111111, -123456.234,
        +3.33, +4.44, +1.11,-2.22 ],

  // array of strings
  vec[" ", ' ', 'hello',
        '123hello', "123hello", '-123hello', '+123hello',
        "\12345678hello", "-\12345678hello", 'h123456ello',
        "1234hello", "hello\0world", "NULL", "true",
        "3", "4", '1', '2'],

  // different arrays
  vec[ vec[0], vec[1, 2], vec[-1, -1],
         vec["123"], vec['123'], vec['-123'], vec["-123"],
         vec[true], vec[TRUE], vec[FALSE],
         vec["123hello"], vec["1", "2"], vec['123hello'], dict[12=>"12twelve"],
         vec["3"], vec["4"], vec["1"], vec["2"] ],

  // array of boolean data
  vec[ true, TRUE, false,
         TRUE, 0, FALSE, 1,
         true, TRUE, FALSE,
         0, 1, 1, 0,
         1, TRUE, 0, FALSE],
  
];
 
// looping to test vsprintf() with different unsigned formats from the above $format array
// and with signed and other types of  values from the above $args_array array
$counter = 1;
foreach($args_array as $args) {
  echo "\n-- Iteration $counter --\n";
  var_dump( vsprintf($formats, $args) );
  $counter++;
}

echo "Done";
}
