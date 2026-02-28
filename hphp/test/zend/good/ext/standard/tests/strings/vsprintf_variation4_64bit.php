<?hh
/* Prototype  : string vsprintf(string format, array args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vsprintf() when different int formats and non-int values are passed to
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vsprintf() : int formats and non-integer values ***\n";

// defining array of int formats
$formats = 
  '%d %+d %-d 
   %ld %Ld %4d %-4d
   %10.4d %-10.4d %.4d %04.4d
   %\'#2d %\'2d %\'$2d %\'_2d
   %3$d %4$d %1$d %2$d';

// Arrays of non int values for the format defined in $format.
// Each sub array contains non int values which correspond to each format in $format
$args_array = vec[

  // array of float values
  vec[2.2, .2, 10.2,
        123456.234, 123456.234, -1234.6789, +1234.6789,
        2e10, +2e5, 4e3, 22e+6,
        12345.780, 12.000000011111, -12.00000111111, -123456.234,
        3.33, +4.44, 1.11,-2.22 ],

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

// looping to test vsprintf() with different int formats from the above $format array
// and with non-int values from the above $args_array array
$counter = 1;
foreach($args_array as $args) {
  echo "\n-- Iteration $counter --\n";
  var_dump( vsprintf($formats, $args) );
  $counter++;
}

echo "Done";
}
