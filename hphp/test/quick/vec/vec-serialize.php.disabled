<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function roundtrip($v) {
  var_dump($v);
  $str = serialize($v);
  var_dump($str);
  $v2 = unserialize($str);
  var_dump($v2);
}

function main() {
  roundtrip(vec[]);
  roundtrip(vec[1, 2, 3]);
  roundtrip(vec['a', 'b', 'c', 'd', 'e', 'f', 'g']);
  roundtrip(vec[new stdclass(), true, false, 1.23, null]);
  roundtrip(vec[vec[], vec[100, 200], vec["key1", "key2", "key3"]]);
  roundtrip(vec[[], [111, 222], dict['a' => 50, 'b' => 60, 'c' => 70]]);

  $empty_vec_str = "v:0:{}";
  var_dump(unserialize($empty_vec_str));

  $ref_str = "v:2:{v:1:{i:123;}r:2;}";
  var_dump(unserialize($ref_str));
}

main();
