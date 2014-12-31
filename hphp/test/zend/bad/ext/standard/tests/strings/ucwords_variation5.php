<?php
/* Prototype  : string ucwords ( string $str )
 * Description: Uppercase the first character of each word in a string
 * Source code: ext/standard/string.c
*/

echo "*** Testing ucwords() : usage variations ***\n";

var_dump(ucwords('testing-dashed-words', '-'));
var_dump(ucwords('test(braced)words', '()'));
var_dump(ucwords('testing empty delimiters', ''));
var_dump(ucwords('testing ranges', 'a..e'));

echo "Done\n";
?>
