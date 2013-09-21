<?php

/* Prototype  : string hebrevc  ( string $hebrew_text  [, int $max_chars_per_line  ] )
 * Description: Convert logical Hebrew text to visual text
 * Source code: ext/standard/string.c
*/

echo "*** Testing hebrevc() : basic functionality ***\n";

$hebrew_text = "The hebrevc function converts logical Hebrew text to visual text.\nThis function is similar to hebrev() with the difference that it converts newlines (\n) to '<br>\n'.\nThe function tries to avoid breaking words.\n";

var_dump(hebrevc($hebrew_text));
var_dump(hebrevc($hebrew_text, 15));

?>
===DONE===