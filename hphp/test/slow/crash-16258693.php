<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function func1($a) { return $a; }
function func2(inout $a) { return $a; }

function vec_foo($f, $k) {
  $d = vec['abc', 'def'];
  return $f($d[$k]);
}

function dict_foo($f, $k) {
  $d = dict[
    1 => 'abc',
    2 => 'def',
  ];
  return $f($d[$k]);
}

function keyset_foo($f, $k) {
  $d = keyset[1, 2];
  return $f($d[$k]);
}


<<__EntryPoint>>
function main_crash_16258693() {
var_dump(vec_foo(fun('func1'), 1));
var_dump(dict_foo(fun('func1'), 2));
var_dump(keyset_foo(fun('func1'), 2));
}
