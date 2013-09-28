<?php

$v = 2147483647;

var_dump(substr("abcde", 1, $v));
var_dump(substr_replace("abcde", "x", $v, $v));

var_dump(strspn("abcde", "abc", $v, $v));
var_dump(strcspn("abcde", "abc", $v, $v));

var_dump(substr_count("abcde", "abc", $v, $v));
var_dump(substr_compare("abcde", "abc", $v, $v));

var_dump(stripos("abcde", "abc", $v));
var_dump(substr_count("abcde", "abc", $v, 1));
var_dump(substr_count("abcde", "abc", 1, $v));
var_dump(strpos("abcde", "abc", $v));
var_dump(stripos("abcde", "abc", $v));
var_dump(strrpos("abcde", "abc", $v));
var_dump(strripos("abcde", "abc", $v));
var_dump(strncmp("abcde", "abc", $v));
var_dump(chunk_split("abcde", $v, "abc"));
var_dump(substr("abcde", $v, $v));

?>