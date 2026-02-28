<?hh
/* Prototype  : string nl2br(string $str);
 * Description: Inserts HTML line breaks before all newlines in a string
 * Source code: ext/standard/string.c
*/

/* Test nl2br() function by passing double quoted strings containing various 
 *   combinations of new line chars to 'str' argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing nl2br() : usage variations ***\n";

$strings = vec[
  //new line chars embedded in strings
  "Hello\nWorld",
  "\nHello\nWorld\n",
  "Hello\rWorld",
  "\rHello\rWorld\r",
  "Hello\r\nWorld",
  "\r\nHello\r\nWorld\r\n",

  //one blank line 
  "
",

  //two blank lines
  "

",

  //inserted new line in a string
  "Hello
World"
];

//loop through $strings array to test nl2br() function with each element
$count = 1;
foreach( $strings as $str ){
  echo "-- Iteration $count --\n";
  var_dump(nl2br($str) );
  $count ++ ;
}
echo "Done";
}
