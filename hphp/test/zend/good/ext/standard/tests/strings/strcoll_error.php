<?php
/* Prototype: int strcoll  ( string $str1  , string $str2  )
   Description: Locale based string comparison
*/

echo "*** Testing strcoll() : error conditions ***\n";

echo "\n-- Testing strcoll() function with no arguments --\n";
var_dump( strcoll() );
var_dump( strcoll("") );

echo "\n-- Testing strcoll() function with one argument --\n";
var_dump( strcoll("Hello World") );  

echo "\n-- Testing strcoll() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( strcoll("Hello World",  "World", $extra_arg) );

?>
===Done===