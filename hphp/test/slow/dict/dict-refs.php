<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$d) {}

function ref_param($d) {
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
    pass_by_ref($d["key"]);
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
    $elem = &$d["key"];
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
  $value = "some-value";
  try {
    $d[] = &$value;
  } catch (Exception $e) {
    echo "append_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($d);
}

function set_ref($d) {
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
    $d["key"] = &$value;
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

function convert_with_ref() {
  $arr = ['a', 'b', 'c', 'd', 'e'];
  $ref = &$arr[3];
  try {
    $d = dict($arr);
    var_dump($d);
  } catch (Exception $e) {
    echo "convert_with_ref exception: \"", $e->getMessage(), "\"\n";
  }
}

function ref_unserialize() {
  $ref_str = "D:2:{s:3:\"foo\";D:1:{s:1:\"a\";s:1:\"b\";}s:3:\"bar\";R:2;}";
  var_dump(unserialize($ref_str));
}

function main() {
  $d = dict[0 => 1, 1 => 2, 2=> 3, 3 => 4];
  ref_param($d);
  elem_ref($d);
  append_ref($d);
  set_ref($d);
  convert_with_ref();
  ref_unserialize();
}
main();
