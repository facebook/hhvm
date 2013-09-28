<?php

/* Prototype: string substr( string str, int start[, int length] )
 * Description: Returns the portion of string specified by the start and length parameters.
 */

$strings_array = array( NULL, "", 12345, "abcdef", "123abc", "_123abc");


/* Testing for error conditions */
echo "*** Testing for error conditions ***\n";

/* Zero Argument */
var_dump( substr() );

/* NULL as Argument */
var_dump( substr(NULL) );

/* Single Argument */
var_dump( substr("abcde") );

/* Scalar Argument */
var_dump( substr(12345) );

/* more than valid number of arguments ( valid are 2 or 3 ) */
var_dump( substr("abcde", 2, 3, 4) );

$counter = 1;
foreach ($strings_array as $str) {
  /* variations with two arguments */
  /* start values >, < and = 0    */

  echo ("\n--- Iteration ".$counter." ---\n");
  echo ("\n-- Variations for two arguments --\n");
  var_dump ( substr($str, 1) );
  var_dump ( substr($str, 0) );
  var_dump ( substr($str, -2) );

  /* variations with three arguments */
  /* start value variations with length values  */

  echo ("\n-- Variations for three arguments --\n");
  var_dump ( substr($str, 1, 3) );
  var_dump ( substr($str, 1, 0) );
  var_dump ( substr($str, 1, -3) );
  var_dump ( substr($str, 0, 3) );
  var_dump ( substr($str, 0, 0) );
  var_dump ( substr($str, 0, -3) );
  var_dump ( substr($str, -2, 3) );
  var_dump ( substr($str, -2, 0 ) );
  var_dump ( substr($str, -2, -3) );

  $counter++;
}

/* variation of start and length to point to same element */
echo ("\n*** Testing for variations of start and length to point to same element ***\n");
var_dump (substr("abcde" , 2, -2) );
var_dump (substr("abcde" , -3, -2) );

/* Testing to return empty string when start denotes the position beyond the truncation (set by negative length) */
echo ("\n*** Testing for start > truncation  ***\n");
var_dump (substr("abcdef" , 4, -4) );

/* String with null character */
echo ("\n*** Testing for string with null characters ***\n");
var_dump (substr("abc\x0xy\x0z" ,2) );

/* String with international characters */
echo ("\n*** Testing for string with international characters ***\n");
var_dump (substr('\xIñtërnâtiônàlizætiøn',3) );

/* start <0 && -start > length */
echo "\n*** Start before the first char ***\n";
var_dump (substr("abcd" , -8) );
 
echo"\nDone";

?>