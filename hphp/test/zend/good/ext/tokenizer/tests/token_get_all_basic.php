<?php
/* Prototype  : array token_get_all(string $source)
 * Description : splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

echo "*** Testing token_get_all() : basic functionality ***\n";

// with php open/close tags
$source = '<?php echo "Hello World"; ?>';
echo "-- source string with PHP open and close tags --\n";
var_dump( token_get_all($source) );

// without php open/close tags testing for T_INLINE_HTML
$source = "echo 'Hello World';";
echo "-- source string without PHP open and close tags --\n";
var_dump( token_get_all($source) );

echo "Done"
?>
