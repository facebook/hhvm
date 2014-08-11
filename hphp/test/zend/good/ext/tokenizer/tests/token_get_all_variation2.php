<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Passing 'source' argument with different arithmetic operators to test them for token
 * Arithmetic operators: +, -, *, /, % are not listed as specific operator tokens,
 *    so they are expected to return string - T_STRING
*/

echo "*** Testing token_get_all() : 'source' string with different arithmetic operators ***\n";

// arithmetic operators - '+', '-', '*', '/', '%' 
$source = array (
  '<?php $a = 1 + 2; ?>',
  '<?php $b = $b - 2; ?>',
  '<?php $c = $a * $b; ?>',
  '<?php $a = $b % 2; ?>'
);
for($count = 0; $count < count($source); $count++) {
  echo "-- Iteration ".($count + 1)." --\n";
  var_dump( token_get_all($source[$count]));
}
echo "Done"
?>
