<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Testing token_get_all() with different function keywords
 *   function - T_FUNCTION(333), return - T_RETURN(335)
 *   different functions: 
 *     include() - T_INCLUDE(262), print() - T_PRINT(266), 
 *     isset() - T_ISSET(349), list() - T_LIST(358), 
 *     require() - T_REQUIRE(259), empty() - T_EMPTY(350), 
 *     declare() - T_DECLARE(324), array() - T_ARRAY(359), 
 *      __halt_compiler() - T_HALT_COMPILER(351)
*/

echo "*** Testing token_get_all() : with different function constructs ***\n";

$source = '<?php
declare(VALUE=100);
include("addfile.php");
require("sumfile.php");

function myFunction($a)
{
  if($a % 2)
    return 1;
  else
    exit;
}

$a = VALUE;
$b = 20;
$c = array(1,2);
$b >>= 2;

if($b <= 0)
  die;
else
  print($b);

list($value1,$value2) = $c;
if(empty($value1) && !isset($value1)) {
  myFunction();
}
?>';
$tokens =  token_get_all($source);
var_dump($tokens);

echo "Done";
?>
