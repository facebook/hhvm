<?php
/* Prototype  : mixed prev(array $array_arg)
 * Description: Move array argument's internal pointer to the previous element and return it
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to prev() to test behaviour
 */

echo "*** Testing prev() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing prev() function with Zero arguments --\n";
try { var_dump( prev() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test prev with one more than the expected number of arguments
echo "\n-- Testing prev() function with more than expected no. of arguments --\n";
$array_arg = array(1, 2);
$extra_arg = 10;
try { var_dump( prev(&$array_arg, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
?>
===DONE===
