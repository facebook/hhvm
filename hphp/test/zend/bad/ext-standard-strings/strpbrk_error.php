<?php
/* Prototype  : array strpbrk(string haystack, string char_list)
 * Description: Search a string for any of a set of characters 
 * Source code: ext/standard/string.c
 * Alias to functions: 
 */

echo "*** Testing strpbrk() : error conditions ***\n";

$haystack = 'This is a Simple text.';
$char_list = 'string_val';
$extra_arg = 10;

echo "\n-- Testing strpbrk() function with more than expected no. of arguments --\n";
var_dump( strpbrk($haystack, $char_list, $extra_arg) );

echo "\n-- Testing strpbrk() function with less than expected no. of arguments --\n";
var_dump( strpbrk($haystack) );

echo "\n-- Testing strpbrk() function with empty second argument --\n";
var_dump( strpbrk($haystack, '') );

echo "\n-- Testing strpbrk() function with arrays --\n";
var_dump( strpbrk($haystack, array('a', 'b', 'c') ) );
var_dump( strpbrk(array('foo', 'bar'), 'b') );

?>
===DONE===