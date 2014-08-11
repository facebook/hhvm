<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Testing token_get_all() with source string containing HTML code with PHP
 *   HTML tags are considered to be T_INLINE_HTML(311)
*/

echo "*** Testing token_get_all() : 'source' string with HTML tags ***\n";

$source = '
<html>
<body>
  Testing HTML
</body>
</html>"

<?php 
  echo "php code with HTML";
?>';
var_dump( token_get_all($source));

echo "Done"
?>
