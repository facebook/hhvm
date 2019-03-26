<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$d) {}

function ref_param($d) {
  echo "========== ref_param ===============================\n";
  try {
    pass_by_ref(&$d[1]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$d[10]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$d["key1"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$d["key2"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$d[false]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($d);
}

function convert_with_ref(&$ref, $arr) {
  echo "========== convert_with_ref ========================\n";
  try {
    $d = dict($arr);
    var_dump($d);
  } catch (Exception $e) {
    echo "convert_with_ref exception: \"", $e->getMessage(), "\"\n";
  }
}

function ref_unserialize() {
  echo "========== ref_unserialize =========================\n";
  $ref_str = "D:2:{s:3:\"foo\";D:1:{s:1:\"a\";s:1:\"b\";}s:3:\"bar\";R:2;}";
  var_dump(unserialize($ref_str));
}

// Its fine to have subelements with refs in them
function nested_refs($d, &$ref, $arr) {
  echo "========== nested_refs =============================\n";
  pass_by_ref(&$d[4][0]);
  var_dump($d);

  $converted = dict($arr);
  var_dump($converted);
}

function main() {
  $arr1 = ['a', 'b', 'c', 'd', 'e'];
  $arr2 = ['a', 'b', ['c', 'd', 'e']];
  $d = dict[0 => 1, 1 => 2, 2=> 3, 3 => 4,
            "key1" => "abc", 4 => [5, 6, 7]];
  ref_param($d);
  convert_with_ref(&$arr1[3], $arr1);
  ref_unserialize();
  nested_refs($d, &$arr2[2][0], $arr2);
}
main();
