<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Passing 'source' argument with different assignment operators to test them for tokens
 *  += - T_PLUS_EQUAL(277), -= - T_MINUS-EQUAL(276), 
 *  *= - T_MUL_EQUAL(275), /= - T_DIVIDE_EQUAL(274),
 *  %= - T_MOD_EQUAL(272), &= - T_AND_EQUAL(271),
 *  |= - T_OR_EQUAL(271), ^= - T_EXOR_EQUAL(269),
 *  >>= - T_SR_EQUAL(267), <<= - T_SL_EQUAL(268), .= - T_CONCAT_EQUAL(273)
*/

echo "*** Testing token_get_all() : 'source' string with different assignment operators ***\n";

// assignment operators : '+=', '-=', '*=', '/=', '%=', '&=', '|=', '^=', '>>=', '<<=', '.='
$source = '<?php 
$a = 1, $b = 2;
$c += $b;
$b -= $a;
$a *= 2;
$d /= 10.50;
$a %= 10.50;
$b &= $c;
$c |= 1;
$d ^= 5;
$a >>= 1;
$b <<= 2;
$d .= "hello world";
?>';
var_dump( token_get_all($source));

echo "Done"
?>
