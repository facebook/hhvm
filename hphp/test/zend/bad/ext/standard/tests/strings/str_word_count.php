<?php
error_reporting(E_ALL);
$str = "Hello friend, you're  
    looking          good today!";
$b =& $str;       
var_dump(str_word_count($str, 1));
var_dump(str_word_count($str, 2));
var_dump(str_word_count($str));
var_dump(str_word_count($str, 3)); 
var_dump(str_word_count($str, 123));
var_dump(str_word_count($str, -1));
var_dump(str_word_count($str, 999999999));
var_dump(str_word_count($str, array()));
var_dump(str_word_count($str, $b));
var_dump($str);

$str2 = "F0o B4r 1s bar foo";
var_dump(str_word_count($str2, NULL, "04"));
var_dump(str_word_count($str2, NULL, "01"));
var_dump(str_word_count($str2, NULL, "014"));
var_dump(str_word_count($str2, NULL, array()));
var_dump(str_word_count($str2, NULL, new stdClass));
var_dump(str_word_count($str2, NULL, ""));
var_dump(str_word_count($str2, 1, "04"));
var_dump(str_word_count($str2, 1, "01"));
var_dump(str_word_count($str2, 1, "014"));
var_dump(str_word_count($str2, 1, array()));
var_dump(str_word_count($str2, 1, new stdClass));
var_dump(str_word_count($str2, 1, ""));
var_dump(str_word_count($str2, 2, "04"));
var_dump(str_word_count($str2, 2, "01"));
var_dump(str_word_count($str2, 2, "014"));
var_dump(str_word_count($str2, 2, array()));
var_dump(str_word_count($str2, 2, new stdClass));
var_dump(str_word_count($str2, 2, ""));
var_dump(str_word_count("foo'0 bar-0var", 2, "0"));
var_dump(str_word_count("'foo'", 2));
var_dump(str_word_count("'foo'", 2, "'"));
var_dump(str_word_count("-foo-", 2));
var_dump(str_word_count("-foo-", 2, "-"));

echo "Done\n";
?>