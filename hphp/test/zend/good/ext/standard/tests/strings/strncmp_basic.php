<?php
/* Prototype  : int strncmp ( string $str1, string $str2, int $len );
 * Description: Binary safe case-sensitive string comparison of the first n characters
 * Source code: Zend/zend_builtin_functions.c
*/

echo "*** Testing strncmp() function: basic functionality ***\n";

echo "-- Testing strncmp() with single quoted string --\n";
var_dump( strncmp('Hello', 'Hello', 5) );  //expected: int(0)
var_dump( strncmp('Hello', 'Hi', 5) );  //expected: value < 0
var_dump( strncmp('Hi', 'Hello', 5) );  //expected: value > 0

echo "-- Testing strncmp() with double quoted string --\n";
var_dump( strncmp("Hello", "Hello", 5) );  //expected: int(0)
var_dump( strncmp("Hello", "Hi", 5) );  //expected: value < 0
var_dump( strncmp("Hi", "Hello", 5) );  //expected: value > 0

echo "-- Testing strncmp() with here-doc string --\n";
$str = <<<HEREDOC
Hello
HEREDOC;
var_dump( strncmp($str, "Hello", 5) );  //expected: int(0)
var_dump( strncmp($str, "Hi", 5) );  //expected: value < 0
var_dump( strncmp("Hi", $str, 5) );  //expected: value > 0

echo "*** Done ***";
?>