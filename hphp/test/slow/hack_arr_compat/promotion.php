<?php

function testElemD($arr, $k) {
  $arr[$k]['ElemD']['undef'] = 42;
}

function testNewElem($arr, $k) {
  $arr[$k][][] = 42;
}

function testSetOpElem($arr, $k) {
  $arr[$k]['SetOpElem'] += 42;
  return $arr;
}

function testSetOpNewElem($arr, $k) {
  $arr[$k][] += 42;
  return $arr;
}

function testIncDecElem($arr, $k) {
  $arr[$k]['IncDecElem']++;
  return $arr;
}

function testIncDecNewElem($arr, $k) {
  $arr[$k][]++;
  return $arr;
}

function main() {
  $a = array();

  testElemD($a, 'a');
  testNewElem($a, 'b');
  testSetOpElem($a, 'c');
  testSetOpNewElem($a, 'd');
  testIncDecElem($a, 'e');
  testIncDecNewElem($a, 'f');
}
main();
