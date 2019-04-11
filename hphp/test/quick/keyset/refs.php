<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function convert_with_ref($arr) {
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

<<__EntryPoint>>
function main() {
  $d = 'd';
  $arr1 = ['a', 'b', 'c', &$d, 'e'];
  convert_with_ref($arr1);
  ref_unserialize();
}
