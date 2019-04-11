<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$_) {}

function convert_with_ref($arr) {
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

<<__EntryPoint>>
function main() {
  $dd = 'd';
  $arr1 = ['a', 'b', 'c', &$dd, 'e'];
  $arr2 = ['a', 'b', ['c', 'd', 'e']];
  $d = dict[0 => 1, 1 => 2, 2=> 3, 3 => 4,
            "key1" => "abc", 4 => [5, 6, 7]];
  convert_with_ref($arr1);
  ref_unserialize();
}
