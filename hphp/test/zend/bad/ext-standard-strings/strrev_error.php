<?php
/* Prototype  : string strrev(string $str);
 * Description: Reverse a string 
 * Source code: ext/standard/string.c
*/

echo "*** Testing strrev() : error conditions ***\n";
echo "-- Testing strrev() function with Zero arguments --";
var_dump( strrev() );

echo "\n-- Testing strrev() function with more than expected no. of arguments --";
var_dump( strrev("string", 'extra_arg') );
echo "*** Done ***";
?>