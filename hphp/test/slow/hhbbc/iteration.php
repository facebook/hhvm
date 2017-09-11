<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function get_int() {
  return (int)__hhvm_intrinsics\launder_value(1);
}

function get_sarr() { return [1, 2, 3]; }
function get_arrn() { return [get_int(), get_int(), get_int()]; }
function get_arre() { return []; }
function get_single_arr() { return [10]; }

function get_packedn() {
  $a = [1];
  for ($i = 0; $i < __hhvm_intrinsics\launder_value(3); $i++) {
    $a[] = get_int();
  }
  return $a;
}

function get_map() {
  return ['a' => get_int(), 'b' => get_int(), 'c' => get_int()];
}

function get_mapn() {
  $a = [
    __hhvm_intrinsics\launder_value('a') => get_int(),
    __hhvm_intrinsics\launder_value('b') => get_int(),
    __hhvm_intrinsics\launder_value('c') => get_int()
  ];
  return $a;
}

function get_arr() {
  return __hhvm_intrinsics\launder_value(true) ? get_arre() : get_arrn();
}
function get_nullable_arr() {
  return __hhvm_intrinsics\launder_value(true) ? null : get_arrn();
}
function get_nullable_single_arr() {
  return __hhvm_intrinsics\launder_value(true) ? null : get_single_arr();
}
function get_empty_single_arr() {
  return __hhvm_intrinsics\launder_value(true) ? get_arre() : get_single_arr();
}

function get_null() { return null; }
function get_obj() { return new stdclass; }

function get_anything() { return __hhvm_intrinsics\launder_value([1, 2, 3]); }

function fun1() {
  $sum = 0;
  foreach (get_sarr() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun2() {
  $sum = 0;
  foreach (get_arrn() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun3() {
  $sum = 0;
  foreach (get_arre() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun4() {
  $sum = 0;
  foreach (get_packedn() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun5() {
  $sum = 0;
  foreach (get_map() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun6() {
  $sum = 0;
  foreach (get_mapn() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun7() {
  $sum = 0;
  foreach (get_arr() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun8() {
  $sum = 0;
  foreach (get_nullable_arr() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun9() {
  $sum = 0;
  foreach (get_single_arr() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun10() {
  $sum = 0;
  foreach (get_nullable_single_arr() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun11() {
  $sum = 0;
  foreach (get_empty_single_arr() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun12() {
  $sum = 0;
  foreach (get_null() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun13() {
  $sum = 0;
  foreach (get_obj() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun14() {
  $sum = 0;
  foreach (get_anything() as $v) {
    $sum += $v;
  }
  return $sum;
}

var_dump(fun1());
var_dump(fun2());
var_dump(fun3());
var_dump(fun4());
var_dump(fun5());
var_dump(fun6());
var_dump(fun7());
var_dump(fun8());
var_dump(fun9());
var_dump(fun10());
var_dump(fun11());
var_dump(fun12());
var_dump(fun13());
var_dump(fun14());
