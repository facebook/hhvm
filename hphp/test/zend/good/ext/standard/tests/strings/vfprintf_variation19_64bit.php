<?hh
/* Prototype  : int vfprintf  ( resource $handle  , string $format , array $args  )
 * Description: Write a formatted string to a stream
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vfprintf() : with  white spaces in format strings ***\n";

// initializing the format array
$formats = varray[
  "% d  %  d  %   d",
  "% f  %  f  %   f",
  "% F  %  F  %   F",
  "% b  %  b  %   b",
  "% c  %  c  %   c",
  "% e  %  e  %   e",
  "% u  %  u  %   u",
  "% o  %  o  %   o",
  "% x  %  x  %   x",
  "% X  %  X  %   X",
  "% E  %  E  %   E"
];

// initializing the args array

$args_array = varray[
  varray[111, 222, 333],
  varray[1.1, .2, -0.6],
  varray[1.12, -1.13, +0.23],
  varray[1, 2, 3],
  varray[65, 66, 67],
  varray[2e1, 2e-1, -2e1],
  varray[-11, +22, 33],
  varray[012, -023, +023],
  varray[0x11, -0x22, +0x33],
  varray[0x11, -0x22, +0x33],
  varray[2e1, 2e-1, -2e1]
];


/* creating dumping file */
$data_file = sys_get_temp_dir().'/'.'vfprintf_variation19_64bit.txt';
if (!($fp = fopen($data_file, 'wt')))
   return;

// looping to test vfprintf() with different scientific formats from the above $format array
// and with non-scientific values from the above $args_array array
$counter = 1;
foreach($formats as $format) {
  fprintf($fp, "\n-- Iteration %d --\n",$counter);
  vfprintf($fp,$format, $args_array[$counter-1]);
  $counter++;
}

fclose($fp);
print_r(file_get_contents($data_file));
echo "\n";

unlink($data_file);
echo "===DONE===\n";
}
