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

function main() {
  $invalid = "\0:";
  var_dump(strpbrk('foo:bar', $invalid));

  nulls_in_haystack();
}

main();
