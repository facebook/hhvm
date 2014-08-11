<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Using different types of constants in 'source' string to check them for token
 * integer const - T_LNUMBER(305)
 * float/double/real const - T_DNUMBER(306)
 * string cosnt - T_CONSTANT_ESCAPED_STRING(315)
 * bool const (no tokens specified) - T_UNKNOWN(307)
 * null const (no tokens specified) - T_UNKNOWN(307)
*/

echo "*** Testing token_get_all() :  'source' string with different constants ***\n";

$a = 1;
$b = 0;

$source = array (
  // int const
  '<?php $a = 1 + 034; $b = $a + 0x3F; ?>', 
  
  // float const
  '<?php $a = 0.23E-2 + 0.43e2 + 0.5; ?>',

  // string const
  '<?php $a = "hello ".\'world\'; ?>',  

  // bool const
  "<?php \$a = (\$b)? true : false; ?>",  
  "<?php \$b = (\$a)? FALSE : TRUE; ?>",  

  // null const
  '<?php $b = null | NULL; ?>'
);
for($count = 0; $count < count($source); $count++) {
  echo "-- Iteration ".($count + 1)." --\n";
  var_dump( token_get_all($source[$count]));
}

echo "Done"
?>
