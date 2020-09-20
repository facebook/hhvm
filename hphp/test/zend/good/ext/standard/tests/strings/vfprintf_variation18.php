<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vfprintf() when different scientific formats and non-scientific values are passed to
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : scientific formats and non-scientific values ***\n";

// defining array of non-scientific formats
$formats = 
  '%e %+e %-e 
   %le %Le %4e %-4e
   %10.4e %-10.4e %04e %04.4e
   %\'#2e %\'2e %\'$2e %\'_2e
   %3$e %4$e %1$e %2$e';

// Arrays of non scientific values for the format defined in $format.
// Each sub array contains non scientific values which correspond to each format in $format
$args_array = varray[

  // array of float values
  varray[2.2, .2, 10.2,
        123456.234, 123456.234, -1234.6789, +1234.6789,
        20.00, +212.2, -411000000000, 2212.000000000001,
        12345.780, 12.000000011111, -12.00000111111, -123456.234,
        3.33, +4.44, 1.11,-2.22 ],

  // array of strings
  varray[" ", ' ', 'hello',
        '123hello', "123hello", '-123hello', '+123hello',
        "\12345678hello", "-\12345678hello", '0123456hello', 'h123456ello',
        "1234hello", "hello\0world", "NULL", "true",
        "3", "4", '1', '2'],

  // different arrays
  varray[ varray[0], varray[1, 2], varray[-1, -1],
         varray["123"], varray['123'], varray['-123'], varray["-123"],
         varray[true], varray[false], varray[TRUE], varray[FALSE],
         varray["123hello"], varray["1", "2"], varray['123hello'], darray[12=>"12twelve"],
         varray["3"], varray["4"], varray["1"], varray["2"] ],

  // array of boolean data
  varray[ true, TRUE, false,
         TRUE, 0, FALSE, 1,
         true, false, TRUE, FALSE,
         0, 1, 1, 0,
         1, TRUE, 0, FALSE],
  
];

/* creating dumping file */
$data_file = __SystemLib\hphp_test_tmppath('vfprintf_variation18.txt');
if (!($fp = fopen($data_file, 'wt')))
   return;
   
// looping to test vfprintf() with different scientific formats from the above $format array
// and with non-scientific values from the above $args_array array
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
