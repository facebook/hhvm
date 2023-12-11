<?hh
/* Prototype  : int strripos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of a case-insensitive 'needle' in a 'haystack'
 * Source code: ext/standard/string.c
*/

/* Test strripos() function by passing single quoted strings to 'haystack' & 'needle' arguments */
<<__EntryPoint>> function main(): void {
echo "*** Testing strripos() function: with single quoted strings ***\n";
$haystack = 'Hello,\t\n\0\n  $&!#%()*<=>?@hello123456he \x234 \101 ';
$needles = vec[
          //regular strings
/*1*/      'l',
          'L',
          'HELLO',
          'hEllo',

          //escape characters
/*5*/      '\t',
          '\T',
          '     ',
          '\n',
          '\N',
          '
        ',  //new line

          //nulls
/*11*/      '\0',
          NULL,
          null,

          //boolean false
/*14*/      FALSE,
          false,

          //empty string
/*16*/      '',

          //special chars
/*17*/      ' ',
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

/*29*/      '12345',     //decimal numeric string
          '\x23',    //hexadecimal numeric string
          '#',      //respective ASCII char of \x23
          '\101',      //octal numeric string
          'A',         // respective ASCII char for \101
          '456HEE', //numerics + chars
          42,         //needle as int(ASCII value of '*')
          $haystack  //haystack as needle
];

/* loop through to get the position of the needle in haystack string */
$count = 1;
foreach ($needles as $needle) {
  echo "-- Iteration $count --\n";
  var_dump( strripos($haystack, $needle) );
  var_dump( strripos($haystack, $needle, 1) );
  var_dump( strripos($haystack, $needle, 20) );
  var_dump( strripos($haystack, $needle, -1) );
  $count++;
}
echo "===DONE===\n";
}
