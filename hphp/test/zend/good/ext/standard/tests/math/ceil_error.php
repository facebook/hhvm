<?php
/* Prototype  : float ceil  ( float $value  )
 * Description: Round fractions up.
 * Source code: ext/standard/math.c
 */

echo "*** Testing ceil() :  error conditions ***\n";
$arg_0 = 1.0;
$extra_arg = 1;

echo "\nToo many arguments\n";
try { var_dump(ceil($arg_0, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\nToo few arguments\n";
try { var_dump(ceil()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
?>
===Done===
