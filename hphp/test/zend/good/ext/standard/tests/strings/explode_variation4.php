<?php

/* Prototype  : array explode  ( string $delimiter  , string $string  [, int $limit  ] )
 * Description: Split a string by string.
 * Source code: ext/standard/string.c
*/

echo "*** Testing explode() function: match longer string ***\n";

$pizza  = "piece1 piece2 piece3 piece4 piece5 piece6 p";
$pieces = explode(" p", $pizza);
var_dump($pieces);
?>
===DONE===