<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$v) {}

function ref_param($v) {
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
  $value = "some-value";
  try {
    $v[] = &$value;
  } catch (Exception $e) {
    echo "append_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($v);
}

function set_ref($v) {
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

function convert_with_ref() {
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
  $ref_str = "v:2:{v:1:{i:123;}R:2;}";
  var_dump(unserialize($ref_str));
}

function main() {
  $v = vec[1, 2, 3, 4];
  ref_param($v);
  elem_ref($v);
  append_ref($v);
  set_ref($v);
  convert_with_ref();
  ref_unserialize();
}
main();
