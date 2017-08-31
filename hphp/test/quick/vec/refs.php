<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$v) {}

function &ret_by_ref($v, $key) { return $v[$key]; }

function ref_param($v) {
  echo "========== ref_param ===============================\n";
  try {
    pass_by_ref($v[1]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($v[10]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($v["key"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($v[false]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($v);
}

function elem_ref($v) {
  echo "========== elem_ref ================================\n";
  try {
    $elem = &$v[1];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$v[10];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$v["key"];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$v[false];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($v);
}

function append_ref($v) {
  echo "========== append_ref ==============================\n";
  $value = "some-value";
  try {
    $v[] = &$value;
  } catch (Exception $e) {
    echo "append_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($v);
}

function set_ref($v) {
  echo "========== set_ref =================================\n";
  $value = "some-value";
  try {
    $v[1] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $v[10] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $v["key"] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $v[false] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($v);
}

function ref_return($v) {
  echo "========== ref_return ==============================\n";
  try {
    ret_by_ref($v, 1);
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($v, 10);
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($v, "key");
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($v, false);
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($v);
}

function iterate_by_ref($v) {
  echo "========== iterate_by_ref ==========================\n";
  try {
    foreach ($v as &$value) { var_dump($value); }
  } catch (Exception $e) {
    echo "iterate_by_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    foreach ($v as $key => &$value) {
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
function nested_refs($v) {
  echo "========== nested_refs =============================\n";
  pass_by_ref($v[4][0]);
  $elem = &$v[4][1];
  $value = 100;
  $v[4][2] = &$value;
  $v[4][] = &$value;
  var_dump($v);

  $arr = ['a', 'b', ['c', 'd', 'e']];
  $ref = &$arr[2][0];
  $converted = vec($arr);
  var_dump($converted);
}

function main() {
  $v = vec[1, 2, 3, 4, [5, 6, 7]];
  ref_param($v);
  elem_ref($v);
  append_ref($v);
  set_ref($v);
  ref_return($v);
  iterate_by_ref($v);
  convert_with_ref();
  ref_unserialize();
  nested_refs($v);
}
main();
