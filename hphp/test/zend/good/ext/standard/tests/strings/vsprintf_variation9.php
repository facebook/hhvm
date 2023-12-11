<?hh
/* Prototype  : string vsprintf(string format, array args)
 * Description: Return a formatted string
 * Source code: ext/standard/formatted_print.c
*/

/*
* Test vsprintf() for char formats with an array of chars passed to the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vsprintf() : char formats with char values ***\n";


// defining array of char formats
$formats = vec[
  "%c",
  "%+c %-c %C",
  "%lc %Lc, %4c %-4c",
  "%10.4c %-10.4c %04c %04.4c",
  "%'#2c %'2c %'$2c %'_2c",
  "%c %c %c %c",
  "% %%c c%",
  '%3$c %4$c %1$c %2$c'
];

// Arrays of char values for the format defined in $format.
// Each sub array contains char values which correspond to each format string in $format
$args_array = vec[
  vec[0],
  vec['c', 67, 68],
  vec[' ', " ", -67, +67],
  vec[97, -97, 98, +98],
  vec[97, -97, 98, +98],
  vec[0x123b, 0xfAb, 0123, 012],
  vec[38, -1234, 2345],
  vec[67, 68, 65, 66]

];

// looping to test vsprintf() with different char formats from the above $format array
// and with char values from the above $args_array array
$counter = 1;
foreach($formats as $format) {
  echo "\n-- Iteration $counter --\n";
  var_dump( vsprintf($format, $args_array[$counter-1]) );
  $counter++;
}

echo "Done";
}
