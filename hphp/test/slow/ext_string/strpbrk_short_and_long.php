<?php

function both_short() {
  print __FUNCTION__."\n";
  $haystack = 'foo:bar';
  $needle1 = '12345:';
  $needle2 = '12345';
  var_dump(strpbrk($haystack, $needle1));
  var_dump(strpbrk($haystack, $needle2));
}

function short_needle() {
  print __FUNCTION__."\n";
  $haystack = '12345678901234567890:1337';
  $needle1 = 'abcdef:';
  $needle2 = 'abcdef';
  var_dump(strpbrk($haystack, $needle1));
  var_dump(strpbrk($haystack, $needle2));
}

function short_haystack() {
  print __FUNCTION__."\n";
  $haystack = '12345';
  $needle1 = 'abcdefghijklmnopqrstuvwxyz3';
  $needle2 = 'abcdefghijklmnopqrstuvwxyz';
  var_dump(strpbrk($haystack, $needle1));
  var_dump(strpbrk($haystack, $needle2));
}

function both_long() {
  print __FUNCTION__."\n";
  $haystack = '12345678901234567890';
  $needle1 = 'abcdefghijklmnopqrstuvwxyz9';
  $needle2 = 'abcdefghijklmnopqrstuvwxyz';
  var_dump(strpbrk($haystack, $needle1));
  var_dump(strpbrk($haystack, $needle2));
}

function empties() {
  print __FUNCTION__."\n";
  var_dump(strpbrk('', '12345'));
  var_dump(strpbrk('', '12345678901234567890'));
}

function multiple_matches() {
  var_dump(strpbrk('foo:bar', ':b'));
  var_dump(strpbrk('foo:bar', 'b:'));
}

function restarting_things() {
  var_dump(strpbrk('foo\x00bar\x00waaaaa', 'w'));
  var_dump(strpbrk('foo\x00bar\x00waaaaa', 'r'));
  var_dump(strpbrk('foo\x00bar\x00waaaaaz', 'z'));
}

function main() {
  both_short();
  short_needle();
  short_haystack();
  both_long();
  empties();
  multiple_matches();
  restarting_things();
}

main();
