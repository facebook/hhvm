<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP language tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Testing token_get_all() with following predefined language constants:
 *   __FILE__     - T_FILE
 *   __CLASS__    - T_CLASS_C
 *   __TRAIT__    - T_TRAIT_C
 *   __FUNCTION__ - T_FUNC_C
 *   __LINE__     - T_LINE
 *   __METHOD__   - T_METHOD_C
*/

echo "*** Testing token_get_all() : with language constants ***\n";

// parsing __FILE__ token
echo "-- with FILE --\n";
$source = "<?php 
\$fp =  fopen(__FILE__, 'r');
?>";
var_dump( token_get_all($source));

// parsing __CLASS__, __TRAIT__ and __FUNCTION__ tokens
echo "-- with CLASS, TRAIT and FUNCTION --\n";
$source = '<?php
class MyClass
{
  echo  __CLASS__;
  echo  __TRAIT__;
  function myFunction()
  {  echo  __FUNCTION__; }
}
?>';
var_dump( token_get_all($source));

// parsing __LINE__ and __METHOD__ tokens
echo "-- with LINE and METHOD --\n";
$source = '<?php
  $a = __LINE__;
  $b = $b.__METHOD__;
?>';
var_dump( token_get_all($source));

echo "Done"
?>
