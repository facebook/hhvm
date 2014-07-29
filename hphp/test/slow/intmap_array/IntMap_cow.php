<?php

function cow_str($arr) {
  $arr["string"] = "cow";
  return $arr;
}

function cow_numericStr($arr) {
  $arr["123"] = "cow";
  return $arr;
}

function cow_append($arr) {
  $arr[] = "cow";
  return $arr;
}

function cow_appendRef($arr) {
  $foo = array(1,2,3);
  $arr[] = &$foo;
}

function cow_refInt($arr) {
  $foo = array(1,2,3);
  $arr[10] = &$foo;
}

function cow_refStr($arr) {
  $foo = array(1,2,3);
  $arr["foo"] = &$foo;
}

function cow_foreachByRef($arr) {
  foreach ($arr as &$valByRef) {
    $valByRef = 0;
  }
}

function main() {
  $a = hphp_miarray();
  $b = cow_str($a);
  $a[] = "warning";
  $b[] = "no warning";

  $a = hphp_miarray();
  $b = cow_numericStr($a);
  $a[] = "warning";
  $b[] = "warning";

  $a = hphp_miarray();
  $b = cow_append($a);
  $a[] = "warning";
  $b[] = "no warning";

  $a = hphp_miarray();
  $b = cow_appendRef($a);
  $a[] = "warning";
  $b[] = "no warning";

  $a = hphp_miarray();
  $b = cow_refInt($a);
  $a[] = "warning";
  $b[] = "no warning";

  $a = hphp_miarray();
  $b = cow_refStr($a);
  $a[] = "warning";
  $b[] = "no warning";

  $a = hphp_miarray();
  $a[100000] = "non-empty";
  $b = cow_foreachByRef($a);
  $a[] = "warning";
  $b[] = "no warning";
}

main();
