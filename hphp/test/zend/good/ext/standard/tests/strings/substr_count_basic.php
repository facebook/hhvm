<?php

echo "***Testing basic operations ***\n";
var_dump(@substr_count("", ""));
var_dump(@substr_count("a", ""));
var_dump(@substr_count("", "a"));
var_dump(@substr_count("", "a"));
var_dump(@substr_count("", chr(0)));
$a = str_repeat("abcacba", 100);
var_dump(@substr_count($a, "bca"));
$a = str_repeat("abcacbabca", 100);
var_dump(@substr_count($a, "bca"));
var_dump(substr_count($a, "bca", 200));
var_dump(substr_count($a, "bca", 200, 50));

echo "Done\n";	

?>