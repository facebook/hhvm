<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$v) {}

function ref_param($v) {
  echo "========== ref_param ===============================\n";
  try {
    pass_by_ref(&$v[1]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$v[10]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$v["key"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$v[false]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($v);
}

function convert_with_ref(&$ref, $arr) {
  echo "========== convert_with_ref ========================\n";
  try {
    $v = vec($arr);
    var_dump($v);
  } catch (Exception $e) {
    echo "convert_with_ref exception: \"", $e->getMessage(), "\"\n";
  }
}

function ref_unserialize() {
  echo "========== ref_unserialize =========================\n";
  $ref_str = "v:2:{v:1:{i:123;}R:2;}";
  var_dump(unserialize($ref_str));
}

// Its fine to have subelements with refs in them
function nested_refs($v, &$ref, $arr) {
  echo "========== nested_refs =============================\n";
  pass_by_ref(&$v[4][0]);
  var_dump($v);

  $converted = vec($arr);
  var_dump($converted);
}

function main() {
  $arr1 = ['a', 'b', 'c', 'd', 'e'];
  $arr2 = ['a', 'b', ['c', 'd', 'e']];
  $v = vec[1, 2, 3, 4, [5, 6, 7]];
  ref_param($v);
  convert_with_ref(&$arr1[3], $arr1);
  ref_unserialize();
  nested_refs($v, &$arr2[2][0], $arr2);
}
main();
