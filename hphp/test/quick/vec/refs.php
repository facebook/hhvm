<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$_) {}

function convert_with_ref($arr) {
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

<<__EntryPoint>>
function main() {
  $d = 'd';
  $arr1 = ['a', 'b', 'c', &$d, 'e'];
  convert_with_ref($arr1);
  ref_unserialize();
}
