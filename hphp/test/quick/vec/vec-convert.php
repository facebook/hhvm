<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function convert_from($v) {
  var_dump((array)$v);
  var_dump(dict($v));
  var_dump((bool)$v);
  var_dump((int)$v);
  var_dump((float)$v);
  var_dump((object)$v);
}

function main() {
  var_dump(vec([]));
  var_dump(vec(vec[]));
  var_dump(vec(dict[]));

  var_dump(vec([100, 200, 300, 400]));
  var_dump(vec(['a', 'b', 'c', 'd', 'e', 'f']));

  var_dump(vec(vec[500, 600, 700, 800]));
  var_dump(vec(vec['g', 'h', 'i', 'j', 'k', 'l']));

  var_dump(vec([55 => "value1", 54 => "value2", 53 => "value3"]));
  var_dump(vec(dict[52 => "value4", 51 => "value5", 50 => "value6"]));

  var_dump(vec(["key1" => true, "key2" => false, "key3" => null]));
  var_dump(vec(dict["key4" => 1.23, "key5" => 7.89, "key6" => 5.32]));

  convert_from(vec[]);
  convert_from(vec[1, 2, 3, 4]);
  convert_from(vec['z', 'y', 'x']);
  convert_from(vec[vec[1, 2], vec[3, 4, 5]]);
}

main();
