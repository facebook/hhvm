<?php

/* Prototype  : string bin2hex  ( string $str  )
 * Description: Convert binary data into hexadecimal representation
 * Source code: ext/standard/string.c
*/

echo "*** Testing bin2hex() : basic functionality ***\n";

// array with different values for $string
$strings =  array (

		  //double quoted strings
/*1*/	  "Here is a simple string",
		  "\t This String contains \t\t some control characters\r\n",
		  "\x90\x91\x00\x93\x94\x90\x91\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f",
		  
		   //single quoted strings
/*4*/	  'Here is a simple string',
		  '\t This String contains \t\t some control characters\r\n',
		  '\x90\x91\x00\x93\x94\x90\x91\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f',
);  

// loop through with each element of the $strings array to test bin2hex() function
$count = 1;
foreach($strings as $string) {
  echo "-- Iteration $count --\n";	
  var_dump(bin2hex($string));
  $count ++;
}  
?>
===DONE===