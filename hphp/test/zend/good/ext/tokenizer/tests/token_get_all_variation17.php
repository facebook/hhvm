<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Testing token_get_all() with different exception keywords
 *   try - T_TRY(336), 
 *   catch - T_CATCH(337),
 *   throw - T_THROW(338)
*/

echo "*** Testing token_get_all() : with exception keywords ***\n";

$source = '<?php
function inverse($x)
{
  if($x == 0) {
    throw new Exception("Divison by zero");
  else
    return 1/$x;
}
try {
  echo inverse(0);
  echo inverse(5);
} catch(Exception $e) {
    echo "caught exception:";
}
?>';
$tokens =  token_get_all($source);
var_dump($tokens);

echo "Done"
?>
