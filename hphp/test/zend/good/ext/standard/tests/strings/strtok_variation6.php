<?php
/* Prototype  : string strtok ( str $str, str $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() : with invalid escape sequences in token
*/

echo "*** Testing strtok() : with invalid escape sequences in token ***\n";

// defining arrays for input strings and tokens
$string_array = array(
 		       "khellok worldk",
 		       "\khello\k world\k",
 		       "/khello\k world/k",
 		       "/hellok/ world"
 		     );
$token_array = array( 
		       "k",
		       "/ ",
		       "/k",
		       "\k",
		       "\\\\\\\k\h\\e\l\o\w\r\l\d" 
 		    );

// loop through each element of the array and check the working of strtok()
// when supplied with different string and token values

$counter =1;
foreach( $string_array as $string )  {
  echo "\n--- Iteration $counter ---\n";
  foreach( $token_array as $token )  { 
    var_dump( strtok($string, $token) ); 
    for( $count = 1; $count <=3; $count++ )  {
      var_dump( strtok($token) );
    }
    echo "\n";
  }
  $counter++;
}		      
		      

echo "Done\n";
?>