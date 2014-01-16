<?php
/* Prototype  : proto int similar_text(string str1, string str2 [, float percent])
* Description: Calculates the similarity between two strings
* Source code: ext/standard/string.c
*/
var_dump(similar_text("abcdefgh", "efg"));
var_dump(similar_text("abcdefgh", "mno"));
var_dump(similar_text("abcdefghcc", "c"));
var_dump(similar_text("abcdefghabcdef", "zzzzabcdefggg"));

$percent = 0;
similar_text("abcdefgh", "efg", $percent);
var_dump($percent);
similar_text("abcdefgh", "mno", $percent);
var_dump($percent);
similar_text("abcdefghcc", "c", $percent);
var_dump($percent);
similar_text("abcdefghabcdef", "zzzzabcdefggg", $percent);
var_dump($percent);
?>
===DONE===