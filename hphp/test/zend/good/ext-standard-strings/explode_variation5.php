<?php

/* Prototype  : array explode  ( string $delimiter  , string $string  [, int $limit  ] )
 * Description: Split a string by string.
 * Source code: ext/standard/string.c
*/

echo "*** Testing explode() function: positive and negative limits ***\n";
$str = 'one||two||three||four';

echo "\n-- positive limit --\n";
var_dump(explode('||', $str, 2));

echo "\n-- negative limit (since PHP 5.1) --\n";
var_dump(explode('||', $str, -1));

echo "\n-- negative limit (since PHP 5.1) with null string -- \n";
var_dump(explode('||', "", -1));
?>
===DONE===