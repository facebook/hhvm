<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Passing 'source' argument with different bitwise operators to test them for token
 *  << - T_SL(287)
 *  >> - T_SR(286)
*/

echo "*** Testing token_get_all() : 'source' string with different bitwise operators ***\n";

// bitwise operators : '<<' , '>>'
$source = '<?php
$a = 2, $b = 4;
$a = $a << 2;
$b = $b >> 2;
var_dump($a);
var_dump($b);
?>';
var_dump( token_get_all($source));

echo "Done"
?>
