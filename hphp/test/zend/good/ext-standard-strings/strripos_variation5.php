<?php
/* Prototype  : int strripos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of a case-insensitive 'needle' in a 'haystack'
 * Source code: ext/standard/string.c
*/

/* Test strripos() function by passing heredoc string containing escape chars for haystack 
 *  and with various needles & offsets 
*/

echo "*** Testing strripos() function: with heredoc strings ***\n";
echo "-- With heredoc string containing escape characters --\n";
$control_char_str = <<<EOD
Hello, World\n
Hello\tWorld
EOD;
var_dump( strripos($control_char_str, "\n") );
var_dump( strripos($control_char_str, "\t") );
var_dump( strripos($control_char_str, "\n", 12) );
var_dump( strripos($control_char_str, "\t", 15) );

?>
===DONE===