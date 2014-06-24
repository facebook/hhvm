<?php

function prettify_null($x) {
  return strtr($x, array("\x00" => '<0>'));
}

function nulls_in_haystack() {
  var_dump(
    prettify_null(strpbrk("foo\x00bar\x00waaaaa", "w\x00")));
  var_dump(
    prettify_null(strpbrk("foo\x00bar\x00waaaaa", "\x00r")));
  var_dump(
    prettify_null(strpbrk("foo\x00bar\x00waaaaa", "w\x00r")));
  var_dump(
    prettify_null(strpbrk("foo\x00bar\x00waaaaaz", "\x00z\x00")));
}

function basic_tests() {
  var_dump(prettify_null(strpbrk('foo:bar', "\0:")));

  $invalid = "\0z";
  var_dump(prettify_null(strpbrk('foo:bar'."\0".'hurr', $invalid)));
  var_dump(prettify_null(strpbrk('foo:bazz'."\0".'hurr', $invalid)));
  var_dump(prettify_null('foo:bazz'."\0".'hurr', "\0"));
}

function main() {
  basic_tests();
  nulls_in_haystack();
}

main();
