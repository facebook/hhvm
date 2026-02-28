<?hh
/* Prototype  : int strrpos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of 'needle' in 'haystack'.
 * Source code: ext/standard/string.c
*/

/* Test strrpos() function by passing double quoted strings for 'haystack' & 'needle' arguments */
<<__EntryPoint>> function main(): void {
echo "*** Testing strrpos() function: with double quoted strings ***\n";
$haystack = "Hello,\t\n\0\n  $&!#%()*<=>?@hello123456he \x234 \101 ";
$needle = vec[
  //regular strings
  "l",  
  "L",
  "HELLO",
  "hEllo",

  //escape characters
  "\t",  
  "\T",  //invalid input
  "     ",
  "\n",
  "\N",  //invalid input
  "
",  //new line

  //nulls
  "\0",  
  NULL,
  null,

  //boolean false
  FALSE,  
  false,

  //empty string
  "",

  //special chars
  " ",  
  "$",
  " $",
  "&",
  "!#", 
  "()",
  "<=>", 
  ">",  
  "=>",
  "?",
  "@",
  "@hEllo",

  "12345", //decimal numeric string  
  "\x23",  //hexadecimal numeric string
  "#",  //respective ASCII char of \x23
  "\101",  //octal numeric string
  "A",  //respective ASCII char of \101
  "456HEE",  //numerics + chars
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
