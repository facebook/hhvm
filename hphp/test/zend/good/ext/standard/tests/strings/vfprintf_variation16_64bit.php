<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vfprintf() when different unsigned formats and signed values and other types of values
 * are passed to the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : unsigned formats and signed & other types of values ***\n";

// defining array of unsigned formats
$formats = 
  '%u %+u %-u 
   %lu %Lu %4u %-4u
   %10.4u %-10.4u %.4u 
   %\'#2u %\'2u %\'$2u %\'_2u
   %3$u %4$u %1$u %2$u';

// Arrays of signed and other type of values for the format defined in $format.
// Each sub array contains signed values which correspond to each format in $format
$args_array = varray[

  // array of float values
  varray[+2.2, +.2, +10.2,
        +123456.234, +123456.234, +1234.6789,
        +2e10, +2e12, +22e+12,
        +12345.780, +12.000000011111, -12.00000111111, -123456.234,
        +3.33, +4.44, +1.11,-2.22 ],

  // array of strings
  varray[" ", ' ', 'hello',
        '123hello', "123hello", '-123hello', '+123hello',
        "\12345678hello", "-\12345678hello", 'h123456ello',
        "1234hello", "hello\0world", "NULL", "true",
        "3", "4", '1', '2'],

  // different arrays
  varray[ varray[0], varray[1, 2], varray[-1, -1],
         varray["123"], varray['123'], varray['-123'], varray["-123"],
         varray[true], varray[TRUE], varray[FALSE],
         varray["123hello"], varray["1", "2"], varray['123hello'], darray[12=>"12twelve"],
         varray["3"], varray["4"], varray["1"], varray["2"] ],

  // array of boolean data
  varray[ true, TRUE, false,
         TRUE, 0, FALSE, 1,
         true, TRUE, FALSE,
         0, 1, 1, 0,
         1, TRUE, 0, FALSE],
  
];
 
/* creating dumping file */
$data_file = __SystemLib\hphp_test_tmppath('vfprintf_variation16_64bit.txt');
if (!($fp = fopen($data_file, 'wt')))
   return;

// looping to test vfprintf() with different unsigned formats from the above $format array
// and with signed and other types of  values from the above $args_array array
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
