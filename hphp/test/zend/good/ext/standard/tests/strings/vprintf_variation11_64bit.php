<?hh
/* Prototype  : string vprintf(string format, array args)
 * Description: Output a formatted string
 * Source code: ext/standard/formatted_print.c
*/

/*
 * Test vprintf() when different octal formats and octal values are passed to
 * the '$format' and '$args' arguments of the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : octal formats with octal values ***\n";

// defining array of octal formats
$formats = varray[
  "%o",
  "%+o %-o %O",
  "%lo %Lo, %4o %-4o",
  "%10.4o %-10.4o %04o %04.4o",
  "%'#2o %'2o %'$2o %'_2o",
  "%o %o %o %o",
  "%% %%o %10 o%",
  '%3$o %4$o %1$o %2$o'
];

// Arrays of octal values for the format defined in $format.
// Each sub array contains octal values which correspond to each format string in $format
$args_array = varray[
  varray[00],
  varray[-01, 01, +022],
  varray[-020000000000, 020000000000, 017777777777, -017777777777],
  varray[0123456, 01234567, -01234567, 01234567],
  varray[0111, 02222, -0333333, -044444444],
  varray[0x123b, 0xfAb, 0123, 012],
  varray[01234, 0567, -01234, 02345],
  varray[03, 04, 01, 02]

];

// looping to test vprintf() with different octal formats from the above $formats array
// and with octal values from the above $args_array array
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
