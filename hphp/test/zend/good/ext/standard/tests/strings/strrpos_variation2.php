<?hh
/* Prototype  : int strrpos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of 'needle' in 'haystack'.
 * Source code: ext/standard/string.c
*/

/* Test strrpos() function by passing single quoted strings to 'haystack' & 'needle' arguments */
<<__EntryPoint>> function main(): void {
echo "*** Testing strrpos() function: with single quoted strings ***\n";
$haystack = 'Hello,\t\n\0\n  $&!#%()*<=>?@hello123456he \x234 \101 ';
$needle = vec[
  //regular strings
  'l',  
  'L',
  'HELLO',
  'hEllo',

  //escape characters
  '\t',  
  '\T',
  '     ',
  '\n',
  '\N',
  '
',  //new line

  //nulls
  '\0',  
  NULL,
  null,

  //boolean false
  FALSE,  
  false,

  //empty string
  '',

  //special chars
  ' ',  
  '$',
  ' $',
  '&',
  '!#',
  '()',
  '<=>',  
  '>',
  '=>',
  '?',
  '@',
  '@hEllo',

  '12345', //decimal numeric string  
  '\x23',  //hexadecimal numeric string
  '#',  //hexadecimal numeric string
  '\101',  //octal numeric string
  'A',
  '456HEE',  //numerics + chars
  42, //needle as int(ASCII value of '*')
  $haystack  //haystack as needle  
];

/* loop through to get the position of the needle in haystack string */
$count = 1;
for($index=0; $index<count($needle); $index++) {
  echo "-- Iteration $count --\n";
  var_dump( strrpos($haystack, $needle[$index]) );
  var_dump( strrpos($haystack, $needle[$index], $index) );
  $count++;
}
echo "*** Done ***";
}
