<?hh
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string.
 * Source code: ext/standard/string.c
*/

/* Test strrchr() function by passing various double quoted strings for 'haystack' & 'needle' */
<<__EntryPoint>> function main(): void {
echo "*** Testing strrchr() function: with various double quoted strings ***";
$haystack = "Hello,\t\n\0\n  $&!#%\o,()*+-./:;<=>?@hello123456he \x234 \101 ";
$needle = vec[
  //regular strings
  "l",
  "L",
  "HELLO",
  "hEllo",

  //escape characters
  "\t",
  "\T",
  "	",
  "\n",
  "\N",
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
  "%\o",
  "\o,",
  "()",
  "*+",
  "+",
  "-",
  ".",
  ".;",
  ":;",
  ";",
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
  42, //needle as int(ASCII value of "*")
  $haystack  //haystack as needle
];

/* loop through to get the position of the needle in haystack string */
$count = 1;
for($index=0; $index<count($needle); $index++) {
  echo "\n-- Iteration $count --\n";
  var_dump( strrchr($haystack, $needle[$index]) );
  $count++;
}
echo "*** Done ***";
}
