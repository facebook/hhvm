<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Passing 'source' argument with different type casting operators to test them for token
 *  (int)/(integer) - T_INT_CAST(295), (float)/(real)/(double) - T_DOUBLE_CAST(294),
 *  (string) - T_STRING_CAST(293), (bool)/(boolean) - T_BOOL_CAST(290),
 *  (unset) - T_UNSET_CAST(289)
*/

echo "*** Testing token_get_all() : 'source' string with different type casting operators ***\n";

// type casting operators : (int), (integer), (float), (real), (double), (string), (array), (object), (bool), (boolean),(unset)
$source = '<?php 
$a = 1, $b = 10.5
$c = (int)$b + $a;
$d = (float)$a + $b;
$e = (string)$a.(string)$b;
if((bool)$a) echo "true";
if(!(boolean)$b) echo "false";
$c = $c + (integer) 123.4e2;
$d = $c - (real) 12;
$b = (unset)$a;
?>';
var_dump( token_get_all($source));

echo "Done"
?>
