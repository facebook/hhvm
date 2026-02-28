<?hh
/* Prototype  : string nl2br(string $str);
 * Description: Inserts HTML line breaks before all newlines in a string
 * Source code: ext/standard/string.c
*/

/* Test nl2br() function by passing single quoted strings containing various
 *   combinations of new line chars to 'str' argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing nl2br() : usage variations ***\n";
$strings = vec[
  '\n',
  '\r',
  '\r\n',
  'Hello\nWorld',
  'Hello\rWorld',
  'Hello\r\nWorld',

  //one blank line
  '
',

  //two blank lines
  '

',

  //inserted new line
  'Hello
World'
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
