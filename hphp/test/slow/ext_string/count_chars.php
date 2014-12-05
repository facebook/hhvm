<?php

class CountCharsTest {
  function __toString() {
    return 'hhvm';
  }
}

echo "mode 0\n";
var_dump(array_filter(count_chars('', 0)));
var_dump(array_filter(count_chars('hhvm', 0)));
var_dump(array_filter(count_chars(new CountCharsTest, 0)));

echo "\nmode 1\n";
var_dump(count_chars('', 1));
var_dump(count_chars('hhvm', 1));
var_dump(count_chars(new CountCharsTest, 1));

echo "\nmode 2\n";
var_dump(count(count_chars('', 2)));
var_dump(count(count_chars('hhvm', 2)));
var_dump(count(count_chars(new CountCharsTest, 2)));

echo "\nmode 3\n";
var_dump(count_chars('', 3));
var_dump(count_chars('hhvm', 3));
var_dump(count_chars(new CountCharsTest, 3));

echo "\nmode 4\n";
var_dump(count_chars('', 4));
var_dump(count_chars('hhvm', 4));
var_dump(count_chars(new CountCharsTest, 4));
