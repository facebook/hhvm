<?hh
/* Prototype  : string vprintf(string format, array args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vprintf() when different float formats and non-float values are passed to
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : float formats and non-float values ***\n";

// defining array of float formats
$formats = 
  '%f %+f %-f 
   %lf %Lf %4f %-4f
   %10.4f %-10.4f %04f %04.4f
   %\'#2f %\'2f %\'$2f %\'_2f
   %3$f %4$f %1$f %2$f';

// Arrays of non float values for the format defined in $format.
// Each sub array contains non float values which correspond to each format in $format
$args_array = varray[

  // array of int values
  varray[2, -2, +2,
        123456, 123456234, -12346789, +12346789,
        123200, +20000, -40000, 22212,
        12345780, 1211111, -12111111, -12345634,
        3, +4, 1,-2 ],

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
 
// looping to test vprintf() with different float formats from the above $format array
// and with non-float values from the above $args_array array
$counter = 1;
foreach($args_array as $args) {
  echo "\n-- Iteration $counter --\n";
  $result = vprintf($formats, $args);
  echo "\n";
  var_dump($result);
  $counter++;
}

echo "===DONE===\n";
}
