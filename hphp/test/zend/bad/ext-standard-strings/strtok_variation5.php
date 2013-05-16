<?php
/* Prototype  : string strtok ( str $str, str $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() : with miscellaneous combinations of string and token
*/

echo "*** Testing strtok() : with miscellaneous inputs ***\n";

// defining arrays for input strings and tokens
$string_array = array(
   		       "HELLO WORLD",
 		       "hello world",
   		       "_HELLO_WORLD_",
		       "/thello/t/wor/ttld",
		       "hel/lo/t/world",
                       "one:$:two:!:three:#:four",
		       "\rhello/r/wor\rrld",
	               chr(0),
                       chr(0).chr(0),
                       chr(0).'hello'.chr(0),
                       'hello'.chr(0).'world'
 		     );
$token_array = array( 
		      "wr",
		      "hello world",
		      "__",
                      "t/",
		      '/t',
		      ":",
		      "\r",
		      "\0",
		      "\0",
		      "\0",
		      "\0",
 		    );

// loop through each element of the array and check the working of strtok()
// when supplied with different string and token values

$counter =1;
foreach( $string_array as $string )  {
  echo "\n--- Iteration $counter ---\n";
  var_dump( strtok($string, $token_array[$counter-1]) ); 
  for( $count = 1; $count <=5; $count++ )  {
    var_dump( strtok($token_array[$counter-1]) );
  }
  $counter++;
}		      
		      

echo "Done\n";
?>