<?php
var_dump(mb_substr_count("abcabcabca", "abcabc"));
var_dump(mb_substr_count("abcabcabca", "abc"));
var_dump(mb_substr_count("abcabcabca", "cab"));
var_dump(mb_substr_count("abcabcabca", "bca"));
var_dump(mb_substr_count("ababababab", "ba"));
var_dump(mb_substr_count("ababababab", "ab"));
var_dump(mb_substr_count("ababababab", "bc"));
var_dump(mb_substr_count("aaaaaaaaaa", "a"));
var_dump(mb_substr_count("aaaaaaaaaa", "b"));
?>