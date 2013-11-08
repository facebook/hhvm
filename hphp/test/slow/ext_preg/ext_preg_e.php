<?php

function test_preg_rep($a,$b,$c) {
  return strtoupper($c).$a;
}

function test_preg_replace() {
  var_dump(preg_replace(
    "/no match/e",
    "strtoupper(\"$1\")",
    "doesn't match"
  ));

  var_dump(preg_replace(
    "/no match/e",
    "bad_func(\"$1\")",
    "doesn't match"
  ));

  var_dump(preg_replace(
    "/(<\\/?\\w+[^>]*>)/e",
    "strtoupper(\"$1\")",
    "<html><body></body></html>"
  ));

  var_dump(preg_replace(
    "/#([A-Fa-f0-9]{3,6});/e",
    "strtolower(\"#\\1;\");",
    "#AAAA;"
  ));

  var_dump(preg_replace(
    "/rgb\\(([0-9]{1,3}), ([0-9]{1,3}), ([0-9]{1,3})\\)/e",
    "sprintf(\"%02x%02x%02x\", \"\\1\", \"\\2\", \"\\3\")",
    "rgb(13, 14, 15)"
  ));

  var_dump(preg_replace(
    "/(a*)(b*)/e",
    "test_preg_rep(\"\\1\",\"smu\\\"rf\",\"\\2\")",
    "aaabbbblahblahaabbbababab"
  ));

  var_dump(preg_replace_callback(
    "/a/e",
    "test_preg_rep",
    "a"
  ));
}

test_preg_replace();
