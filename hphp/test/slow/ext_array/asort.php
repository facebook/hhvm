<?php

$fruits = array(
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
);

asort($fruits);
var_dump($fruits);

$arr = array("at", "\xe0s", "as");
asort($arr, 0);
$arr = array("num2ber", "num1ber", "num10ber");
asort($arr, SORT_REGULAR);
var_dump($arr);

$arr = array("G\xediron",        // &iacute; (Latin-1)
                     "G\xc3\xb3nzales",  // &oacute; (UTF-8)
                     "G\xc3\xa9 ara",    // &eacute; (UTF-8)
                     "G\xe1rcia");       // &aacute; (Latin-1)
asort($arr, SORT_REGULAR);

# Make the output ASCII-safe
foreach($arr as &$val) {
  $val = urlencode($val);
}
unset($val);
var_dump($arr);
