<?php
/* Prototype  : proto int similar_text(string str1, string str2 [, float percent])
* Description: Calculates the similarity between two strings
* Source code: ext/standard/string.c
*/

$extra_arg = 10;
echo "\n-- Testing similar_text() function with more than expected no. of arguments --\n";
similar_text("abc", "def", $percent, $extra_arg);

echo "\n-- Testing similar_text() function with less than expected no. of arguments --\n";
similar_text("abc");
?>
===DONE===