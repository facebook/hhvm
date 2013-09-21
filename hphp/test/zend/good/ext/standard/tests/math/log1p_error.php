<?php
/* Prototype  : float log1p  ( float $arg  )
 * Description: Returns log(1 + number), computed in a way that is accurate even 
 *				when the value of number is close to zero
 * Source code: ext/standard/math.c
 */
 
echo "*** Testing log1p() : error conditions ***\n";

echo "\n-- Testing log1p() function with less than expected no. of arguments --\n";
log1p();
echo "\n-- Testing log1p() function with more than expected no. of arguments --\n";
log1p(36, true);
?>
===Done===