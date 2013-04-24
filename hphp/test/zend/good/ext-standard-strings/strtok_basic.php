<?php
/* Prototype  : string strtok ( str $str, str $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() : basic functionality
*/

echo "*** Testing strtok() : basic functionality ***\n";

// Initialize all required variables
$str = 'This testcase test strtok() function.';
$token = ' ().';

echo "\nThe Input string is:\n\"$str\"\n";
echo "\nThe token string is:\n\"$token\"\n";

// using strtok() with $str argument
echo "\n--- Token 1 ---\n";
var_dump( strtok($str, $token) );

for( $i = 2; $i <=7; $i++ )  {
  echo "\n--- Token $i ---\n";
  var_dump( strtok($token) );
}

echo "Done\n";
?>