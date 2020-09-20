<?hh
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sprintf() : unsigned formats with integer values ***\n";

// array of integer values 
$integer_values = varray[
  0,
  1,
  -1,
  -2147483648, // max negative integer value
  -2147483647,
  2147483647,  // max positive integer value
  +2147483640,
  0x123B,      // integer as hexadecimal
  0x12ab,
  0Xfff,
  0XFA,
  -0x80000000, // max negative integer as hexadecimal
  0x7fffffff,  // max postive integer as hexadecimal
  0x7FFFFFFF,  // max postive integer as hexadecimal
  0123,        // integer as octal 
  01912,       // should be quivalent to octal 1
  -020000000000, // max negative integer as octal 
  017777777777  // max positive integer as octal
];

// array of unsigned formats
$unsigned_formats = varray[
  "%u", "%hu", "%lu",
  "%Lu", " %u", "%u ",
  "\t%u", "\n%u", "%4u",
  "%30u", "%[0-9]", "%*u"
];


$count = 1;
foreach($integer_values as $integer_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($unsigned_formats as $format) {
    var_dump( sprintf($format, $integer_value) );
  }
  $count++;
};

echo "Done";
}
