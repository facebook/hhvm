<?php

/* Prototype  : int strlen  ( string $string  )
 * Description: Get string length
 * Source code: ext/standard/string.c
*/

echo "*** Testing strlen() : unexpected number of arguments ***";


echo "\n-- Testing strlen() function with no arguments --\n";
try { var_dump( strlen() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strlen() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( strlen("abc def", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
?>
===DONE===
