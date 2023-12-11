<?hh

function prettify_null($x) :mixed{
  return strtr($x, dict["\x00" => '<0>']);
}

function nulls_in_haystack() :mixed{
  var_dump(
    prettify_null(strpbrk("foo\x00bar\x00waaaaa", "w\x00")));
  var_dump(
    prettify_null(strpbrk("foo\x00bar\x00waaaaa", "\x00r")));
  var_dump(
    prettify_null(strpbrk("foo\x00bar\x00waaaaa", "w\x00r")));
  var_dump(
    prettify_null(strpbrk("foo\x00bar\x00waaaaaz", "\x00z\x00")));
}

function basic_tests() :mixed{
  var_dump(prettify_null(strpbrk('foo:bar', "\0:")));

  $invalid = "\0z";
  var_dump(prettify_null(strpbrk('foo:bar'."\0".'hurr', $invalid)));
  var_dump(prettify_null(strpbrk('foo:bazz'."\0".'hurr', $invalid)));
  var_dump(prettify_null('foo:bazz'."\0".'hurr', "\0"));
}

function main() :mixed{
  basic_tests();
  nulls_in_haystack();
}


<<__EntryPoint>>
function main_strpbrk_null() :mixed{
main();
}
