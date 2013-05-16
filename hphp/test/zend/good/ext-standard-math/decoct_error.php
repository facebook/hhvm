<?php
/* Prototype  : string decbin  ( int $number  )
 * Description: Decimal to binary.
 * Source code: ext/standard/math.c
 */

echo "*** Testing decoct() :  error conditions ***\n";

echo "Incorrect number of arguments\n";
decoct();
decoct(23,2,true);

?>
===Done===