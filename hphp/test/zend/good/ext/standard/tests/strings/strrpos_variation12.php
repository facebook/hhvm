<?php
/* Prototype  : int strrpos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of 'needle' in 'haystack'.
 * Source code: ext/standard/string.c
*/

/* Test strrpos() function with null terminated strings for 'haystack' argument 
 *  in order to check the binary safe 
*/

echo "*** Test strrpos() function: binary safe ***\n";
$haystacks = array(
  "Hello".chr(0)."World",
  chr(0)."Hello World",
  "Hello World".chr(0),
  chr(0).chr(0).chr(0),
  "Hello\0world",
  "\0Hello",
  "Hello\0"
);

for($index = 0; $index < count($haystacks); $index++ ) {
  var_dump( strrpos($haystacks[$index], "\0") );
  var_dump( strrpos($haystacks[$index], "\0", $index) );
}
echo "*** Done ***";
?>