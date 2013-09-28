<?php
/* Prototype  : string strtok ( string $str, string $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() for error conditions
*/

echo "*** Testing strtok() : error conditions ***\n";

// Zero argument
echo "\n-- Testing strtok() function with Zero arguments --\n";
var_dump( strtok() );

// More than expected number of arguments
echo "\n-- Testing strtok() function with more than expected no. of arguments --\n";
$str = 'sample string';
$token = ' ';
$extra_arg = 10;

var_dump( strtok($str, $token, $extra_arg) );
var_dump( $str );

// Less than expected number of arguments 
echo "\n-- Testing strtok() with less than expected no. of arguments --\n";
$str = 'string val';
 
var_dump( strtok($str));
var_dump( $str );

echo "Done\n";
?>