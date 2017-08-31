<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$d) {}

function &ret_by_ref($d, $key) { return $d[$key]; }

function ref_param($d) {
  echo "========== ref_param ===============================\n";
  try {
    pass_by_ref($d[1]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($d[10]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($d["key1"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($d["key2"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($d[false]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($d);
}

function elem_ref($d) {
  echo "========== elem_ref ================================\n";
  try {
    $elem = &$d[1];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$d[10];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$d["key1"];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$d["key2"];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$d[false];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($d);
}

function append_ref($d) {
  echo "========== append_ref ==============================\n";
  $value = "some-value";
  try {
    $d[] = &$value;
  } catch (Exception $e) {
    echo "append_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($d);
}

function set_ref($d) {
  echo "========== set_ref =================================\n";
  $value = "some-value";
  try {
    $d[1] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $d[10] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $d["key1"] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $d["key2"] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $d[false] = &$value;
  } catch (Exception $e) {
    echo "set_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($d);
}

function ref_return($d) {
  echo "========== ref_return ==============================\n";
  try {
    ret_by_ref($d, 1);
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($d, 10);
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($d, "key1");
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($d, "key2");
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    ret_by_ref($d, false);
  } catch (Exception $e) {
    echo "ref_return exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($d);
}

function convert_with_ref() {
  echo "========== convert_with_ref ========================\n";
  $arr = ['a', 'b', 'c', 'd', 'e'];
  $ref = &$arr[3];
  try {
    $d = dict($arr);
    var_dump($d);
  } catch (Exception $e) {
    echo "convert_with_ref exception: \"", $e->getMessage(), "\"\n";
  }
}

function iterate_by_ref($d) {
  echo "========== iterate_by_ref ==========================\n";
  try {
    foreach ($d as &$value) { var_dump($value); }
  } catch (Exception $e) {
    echo "iterate_by_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    foreach ($d as $key => &$value) {
      var_dump($key);
      var_dump($value);
    }
  } catch (Exception $e) {
    echo "iterate_by_ref (key) exception: \"", $e->getMessage(), "\"\n";
  }
}

function ref_unserialize() {
  echo "========== ref_unserialize =========================\n";
  $ref_str = "D:2:{s:3:\"foo\";D:1:{s:1:\"a\";s:1:\"b\";}s:3:\"bar\";R:2;}";
  var_dump(unserialize($ref_str));
}

// Its fine to have subelements with refs in them
function nested_refs($d) {
  echo "========== nested_refs =============================\n";
  pass_by_ref($d[4][0]);
  $elem = &$d[4][1];
  $value = 100;
  $d[4][2] = &$value;
  $d[4][] = &$value;
  var_dump($d);

  $arr = ['a', 'b', ['c', 'd', 'e']];
  $ref = &$arr[2][0];
  $converted = dict($arr);
  var_dump($converted);
}

function main() {
  $d = dict[0 => 1, 1 => 2, 2=> 3, 3 => 4,
            "key1" => "abc", 4 => [5, 6, 7]];
  ref_param($d);
  elem_ref($d);
  append_ref($d);
  set_ref($d);
  ref_return($d);
  iterate_by_ref($d);
  convert_with_ref();
  ref_unserialize();
  nested_refs($d);
}
main();
