<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Passing 'source' argument with different logical operators to test them for tokens
 *   and - T_AND_LOGICAL_AND(265), 
 *   or - T_LOGICAL_OR(263), 
 *   xor - T_LOGICAL_XOR(264), 
 *   && - T_BOOLEAN_AND(279), 
 *   || - T_BOOLEAN_OR(278)
*/

echo "*** Testing token_get_all() : 'source' string with different logical operators ***\n";

// logical operators : 'and', 'or', 'xor', '&&', '||' 
$source = array (
  '<?php $a = 1 and 024; ?>',
  '<?php $b = $b or 0X1E; ?>',
  '<?php $c = $a xor $b; ?>',
  '<?php $a = $b && 2; ?>',
  '<?php $b = $b || 1; ?>'
);
for($count = 0; $count < count($source); $count++) {
  echo "-- Iteration ".($count + 1)." --\n";
  var_dump( token_get_all($source[$count]));
}

echo "Done"
?>
