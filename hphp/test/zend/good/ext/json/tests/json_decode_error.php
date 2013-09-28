<?php
/* Prototype  : mixed json_decode  ( string $json  [, bool $assoc=false  [, int $depth=512  ]] )
 * Description: Decodes a JSON string
 * Source code: ext/json/php_json.c
 * Alias to functions: 
 */
echo "*** Testing json_decode() : error conditions ***\n";

echo "\n-- Testing json_decode() function with no arguments --\n";
var_dump( json_decode() );

echo "\n-- Testing json_decode() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( json_decode('"abc"', TRUE, 512, 0, $extra_arg) );

?>
===Done===