<?php
/* Prototype  : string strtok ( str $str, str $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() : modifying the input string while it is getting tokenised
*/

echo "*** Testing strtok() : with modification of input string in between tokenising ***\n";

$str = "this is a sample string";
$token = " ";

echo "\n*** Testing strtok() when string being tokenised is prefixed with another string in between the process ***\n";
var_dump( strtok($str, $token) ); 
// adding a string to the input string which is being tokenised
$str = "extra string ".$str;
for( $count = 1; $count <=6; $count++ )  {
  echo "\n-- Token $count is --\n";
  var_dump( strtok($token) );
  echo "\n-- Input str is \"$str\" --\n";
}
		      
echo "\n*** Testing strtok() when string being tokenised is suffixed with another string in between the process ***\n";
var_dump( strtok($str, $token) ); 
// adding a string to the input string which is being tokenised
$str = $str." extra string";
for( $count = 1; $count <=10; $count++ )  {
  echo "\n-- Token $count is --\n";
  var_dump( strtok($token) );
}

echo "Done\n";
?>