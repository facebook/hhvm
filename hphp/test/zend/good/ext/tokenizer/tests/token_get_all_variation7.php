<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Passing 'source' argument with different increment/decrement operators to test them for token
 *  ++ - T_INC(297)
 *  -- - T_DEC(296)
*/

echo "*** Testing token_get_all() : 'source' string with different increment/decrement operators ***\n";

// increment/decrement operators : '++' , '--'
$source = '<?php 
$a = 10, $b = 5;
$a++;
$b--;
echo $a;
?>';
var_dump( token_get_all($source));

echo "Done"
?>
