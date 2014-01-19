<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function strtobin($str) {
  $ret = '';
  $sep = '';
  for ($i = 0; $i < strlen($str); ++$i) {
    $char = $str[$i];
    $ret .= $sep . $char . ':' . ord($char);
    $sep = ', ';
  }
  return $ret;
}

function do_string($str) {
  var_dump(strtobin($str),
           strtobin(stripcslashes($str)));
}

function main() {
  do_string("12345\\:2\\");
  do_string("12345\\:2\\4");
  do_string("12345\\:a");
  do_string("12345\\123");
  do_string("12345\\12345");
}
main();
