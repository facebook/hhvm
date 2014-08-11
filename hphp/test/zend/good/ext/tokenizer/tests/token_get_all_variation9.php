<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Passing 'source' argument with different style of comments
 */
//  '//', '/* */', '#' - T_COMMENT(365)
// '/** */' - T_DOC_COMMENT(366)


echo "*** Testing token_get_all() : 'source' string with different comments ***\n";

// types of comments: '//', '/* */', '#' & /** */

$source = '<?php 
/** Performing addition operation on given values :
  * a, b
  */

// int value
$a = 10;
$b = 20;
$c = true; // bool value

/* 
 * Performing operation on $a,$b 
 * display result
 */
$c = $a + $b;
var_dump($c); # expected: int(30)

# end of program
?>';
var_dump( token_get_all($source));

echo "Done"
?>
