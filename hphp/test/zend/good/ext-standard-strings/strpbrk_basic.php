<?php
/* Prototype  : array strpbrk(string haystack, string char_list)
 * Description: Search a string for any of a set of characters 
 * Source code: ext/standard/string.c
 * Alias to functions: 
 */

echo "*** Testing strpbrk() : basic functionality ***\n";

// Initialise all required variables
$text = 'This is a Simple text.';
var_dump( strpbrk($text, 'mi') );
var_dump( strpbrk($text, 'ZS') );
var_dump( strpbrk($text, 'Z') );
var_dump( strpbrk($text, 'H') );

$text = '';
var_dump( strpbrk($text, 'foo') );

$text = "  aaa aaaSLR";
var_dump( strpbrk($text, '     ') );

var_dump( strpbrk(5, 5) );
var_dump( strpbrk(5, "5") );

?>
===DONE===