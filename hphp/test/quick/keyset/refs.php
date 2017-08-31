<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$ks) {}

function &ret_by_ref($ks, $key) { return $ks[$key]; }

function ref_param($ks) {
  echo "========== ref_param ===============================\n";
  try {
    pass_by_ref($ks[1]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($ks[10]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($ks["key1"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($ks["key2"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($ks[false]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($ks);
}

function elem_ref($ks) {
  echo "========== elem_ref ================================\n";
  try {
    $elem = &$ks[1];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$ks[10];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$ks["key1"];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$ks["key2"];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$ks[false];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($ks);
}

function append_ref($ks) {
  echo "========== append_ref ==============================\n";
  $ksalue = "some-value";
  try {
    $ks[] = &$value;
  } catch (Exception $e) {
    echo "append_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($ks);
}

function set_ref($ks) {
  echo "========== set_ref =================================\n";
  $value = "some-value";
  try {
    $ks[1] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $ks[10] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $ks["key1"] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $ks["key2"] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $ks[false] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($ks);
}

function ref_return($ks) {
  echo "========== ref_return ==============================\n";
  try {
    ret_by_ref($ks, 1);
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($ks, 10);
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($ks, "key1");
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($ks, "key2");
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($ks, false);
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($ks);
}

function iterate_by_ref($ks) {
  echo "========== iterate_by_ref ==========================\n";
  try {
    foreach ($ks as &$value) { var_dump($value); }
  } catch (Exception $e) {
    echo "iterate_by_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    foreach ($ks as $key => &$value) {
      var_dump($key);
      var_dump($value);
    }
  } catch (Exception $e) {
    echo "iterate_by_ref (key) exception: \"", $e->getMessage(), "\"\n";
  }
}

function convert_with_ref() {
  echo "========== convert_with_ref ========================\n";
  $arr = ['a', 'b', 'c', 'd', 'e'];
  $ref = &$arr[3];
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
  $ks = keyset["key1", 1, "abc", 123];
  ref_param($ks);
  elem_ref($ks);
  append_ref($ks);
  set_ref($ks);
  ref_return($ks);
  iterate_by_ref($ks);
  convert_with_ref();
  ref_unserialize();
}
main();
