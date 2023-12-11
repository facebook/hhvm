<?hh
/* Prototype  : string nl2br(string $str)
 * Description: Inserts HTML line breaks before all newlines in a string.
 * Source code: ext/standard/string.c
*/

/*
* Test nl2br() function by passing html string inputs containing line breaks and
*   new line chars for 'str'
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing nl2br() : usage variations ***\n";

//array of html strings
$strings = vec[
  "<html>Hello<br />world</html>",
  "<html><br /></html>",
  "<html>\nHello\r\nworld\r</html>",
  "<html>\n \r\n \r</html>",
];

//loop through $strings array to test nl2br() function with each element
foreach( $strings as $str ){
  var_dump(nl2br($str) );
}
echo "Done";
}
