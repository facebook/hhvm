<?php
/* Prototype  : float expm1  ( float $arg  )
 * Description: Returns exp(number) - 1, computed in a way that is accurate even 
 *              when the value of number is close to zero.
 * Source code: ext/standard/math.c
 */

echo "*** Testing expm1() : error conditions ***\n";

echo "\n-- Testing expm1() function with less than expected no. of arguments --\n";
expm1();
echo "\n-- Testing expm1() function with more than expected no. of arguments --\n";
expm1(23,true);

?>
===Done===