<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$ks) {}

function ref_param($ks) {
  echo "========== ref_param ===============================\n";
  try {
    pass_by_ref(&$ks[1]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$ks[10]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$ks["key1"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$ks["key2"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref(&$ks[false]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($ks);
}

function convert_with_ref(&$ref, $arr) {
  echo "========== convert_with_ref ========================\n";
  try {
    $ks = keyset($arr);
    var_dump($ks);
  } catch (Exception $e) {
    echo "convert_with_ref exception: \"", $e->getMessage(), "\"\n";
  }
}

function ref_unserialize() {
  echo "========== ref_unserialize =========================\n";
  $ref_str = "a:2:{i:123;s:3:\"abc\";i:456;k:1:{R:2;}}";
  var_dump(unserialize($ref_str));
}

function main() {
  $arr1 = ['a', 'b', 'c', 'd', 'e'];
  $ks = keyset["key1", 1, "abc", 123];
  ref_param($ks);
  convert_with_ref(&$arr1[3], $arr1);
  ref_unserialize();
}
main();
