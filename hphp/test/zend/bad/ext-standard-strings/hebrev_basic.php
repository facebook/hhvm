<?php

/* Prototype  : string hebrev  ( string $hebrew_text  [, int $max_chars_per_line  ] )
 * Description: Convert logical Hebrew text to visual text
 * Source code: ext/standard/string.c
*/

echo "*** Testing hebrev() : basic functionality ***\n";

$hebrew_text = "The hebrev function converts logical Hebrew text to visual text.\nThe function tries to avoid breaking words.\n";

var_dump(hebrev($hebrew_text));
var_dump(hebrev($hebrew_text, 15));

?>
===DONE===