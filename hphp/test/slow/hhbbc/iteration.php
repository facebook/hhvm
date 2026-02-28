<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function get_int() :mixed{
  return (int)__hhvm_intrinsics\launder_value(1);
}

function get_sarr() :mixed{
  return vec[1, 2, 3];
}
function get_arrn() :mixed{
  return vec[get_int(), get_int(), get_int()];
}
function get_arre() :mixed{
  return vec[];
}
function get_single_arr() :mixed{
  return vec[10];
}

function get_packedn() :mixed{
  $a = vec[1];
  for ($i = 0; $i < __hhvm_intrinsics\launder_value(3); $i++) {
    $a[] = get_int();
  }
  return $a;
}

function get_map() :mixed{
  return dict['a' => get_int(), 'b' => get_int(), 'c' => get_int()];
}

function get_mapn() :mixed{
  $a = dict[
    __hhvm_intrinsics\launder_value('a') => get_int(),
    __hhvm_intrinsics\launder_value('b') => get_int(),
    __hhvm_intrinsics\launder_value('c') => get_int(),
  ];
  return $a;
}

function get_arr() :mixed{
  return __hhvm_intrinsics\launder_value(true) ? get_arre() : get_arrn();
}
function get_nullable_arr() :mixed{
  return __hhvm_intrinsics\launder_value(true) ? null : get_arrn();
}
function get_nullable_single_arr() :mixed{
  return __hhvm_intrinsics\launder_value(true) ? null : get_single_arr();
}
function get_empty_single_arr() :mixed{
  return __hhvm_intrinsics\launder_value(true) ? get_arre() : get_single_arr();
}

function get_null() :mixed{
  return null;
}
function get_obj() :mixed{
  return new stdClass;
}

function get_anything() :mixed{
  return __hhvm_intrinsics\launder_value(vec[1, 2, 3]);
}

function fun1() :mixed{
  $sum = 0;
  foreach (get_sarr() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun2() :mixed{
  $sum = 0;
  foreach (get_arrn() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun3() :mixed{
  $sum = 0;
  foreach (get_arre() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun4() :mixed{
  $sum = 0;
  foreach (get_packedn() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun5() :mixed{
  $sum = 0;
  foreach (get_map() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun6() :mixed{
  $sum = 0;
  foreach (get_mapn() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun7() :mixed{
  $sum = 0;
  foreach (get_arr() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun8() :mixed{
  $sum = 0;
  try {
    foreach (get_nullable_arr() as $v) {
      $sum += $v;
    }
  } catch (InvalidForeachArgumentException $e) {
    var_dump($e->getMessage());
  }
}

function fun9() :mixed{
  $sum = 0;
  foreach (get_single_arr() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun10() :mixed{
  $sum = 0;
  try {
    foreach (get_nullable_single_arr() as $v) {
      $sum += $v;
    }
  } catch (InvalidForeachArgumentException $e) {
    var_dump($e->getMessage());
  }
}

function fun11() :mixed{
  $sum = 0;
  foreach (get_empty_single_arr() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun12() :mixed{
  $sum = 0;
  try {
    foreach (get_null() as $v) {
      $sum += $v;
    }
  } catch (InvalidForeachArgumentException $e) {
    var_dump($e->getMessage());
  }
}

function fun13() :mixed{
  $sum = 0;
  foreach (get_obj() as $v) {
    $sum += $v;
  }
  return $sum;
}

function fun14() :mixed{
  $sum = 0;
  foreach (get_anything() as $v) {
    $sum += $v;
  }
  return $sum;
}


<<__EntryPoint>>
function main_iteration() :mixed{
  var_dump(fun1());
  var_dump(fun2());
  var_dump(fun3());
  var_dump(fun4());
  var_dump(fun5());
  var_dump(fun6());
  var_dump(fun7());
  fun8();
  var_dump(fun9());
  fun10();
  var_dump(fun11());
  fun12();
  var_dump(fun13());
  var_dump(fun14());
}
