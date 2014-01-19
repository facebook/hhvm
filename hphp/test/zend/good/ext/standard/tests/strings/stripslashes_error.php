<?php
/* Prototype  : string stripslashes ( string $str )
 * Description: Returns an un-quoted string
 * Source code: ext/standard/string.c
*/

/*
 * Testing stripslashes() for error conditions
*/

echo "*** Testing stripslashes() : error conditions ***\n";

// Zero argument
echo "\n-- Testing stripslashes() function with Zero arguments --\n";
var_dump( stripslashes() );

// More than expected number of arguments
echo "\n-- Testing stripslashes() function with more than expected no. of arguments --\n";
$str = '\"hello\"\"world\"';
$extra_arg = 10;

var_dump( stripslashes($str, $extra_arg) );
var_dump( $str );

echo "Done\n";
?>