<?php

var_dump(substr_compare("abcde", "bc", 1, 2));
var_dump(substr_compare("abcde", "bcg", 1, 2));
var_dump(substr_compare("abcde", "BC", 1, 2, true));
var_dump(substr_compare("abcde", "bc", 1, 3));
var_dump(substr_compare("abcde", "cd", 1, 2));
var_dump(substr_compare("abcde", "abc", 5, 1));
var_dump(substr_compare("abcde", "abcdef", -10, 10));

var_dump(substr_compare("abcde", -1, 0, NULL, new stdClass));
echo "Test\n";
var_dump(substr_compare("abcde", "abc", -1, NULL, -5));
var_dump(substr_compare("abcde", -1, 0, "str", new stdClass));

echo "Done\n";
?>