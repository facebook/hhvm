<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Passing 'source' argument with different comparison operators to test them for tokens
 *  == - T_IS_EQUAL(283), === - T_IS_IDENTICAL(281), 
 *  >= - T_IS_GREATER_OR_EQUAL(284), <= - T_IS_LESS_OR_EQUAL(285)
 *  != - T_IS_NOT_EQUAL, <> - T_IS_NOT_EQUAL(282), !== - T_IS_NOT_IDENTICAL(280)
*/

echo "*** Testing token_get_all() : 'source' string with different comparison operators ***\n";

// comparison operators : '==', '===', '>=', '<=', '!=', '!==', '<>'
$source = '<?php 
if($a == 0) 
  echo "== 0";
elseif($a === 2)
  echo "=== 2";
elseif($a >= 10 && $a <= 20)
  echo ">= 10 & <=20";
elseif($a != 1 || $a <> 1)
  echo "!= 1";
elseif($a !== 1)
  echo "!==1";
?>';
var_dump( token_get_all($source));

echo "Done"
?>
