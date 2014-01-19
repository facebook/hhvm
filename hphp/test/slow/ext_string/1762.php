<?php

var_dump(explode('', ''));
$str = 'Hello Friend';
var_dump(str_split($str, -3));
var_dump(chunk_split('-=blender=-', -3, '-=blender=-'));
 var_dump(strpbrk('hello', ''));
var_dump(substr_count('hello', ''));
var_dump(substr_count('hello', 'o', -1));
var_dump(substr_count('hello', 'o', 2, -1));
var_dump(substr_count('hello', 'o', 2, 100));
var_dump(count_chars('hello', 100));
var_dump(str_word_count('hello', 100));
var_dump(strtr('hello', 100));
var_dump(implode('abcd', 'abcd'));
