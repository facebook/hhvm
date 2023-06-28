<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function func1($a) :mixed{ return $a; }
function func2(inout $a) :mixed{ return $a; }

function vec_foo($f, $k) :mixed{
  $d = vec['abc', 'def'];
  return $f($d[$k]);
}

function dict_foo($f, $k) :mixed{
  $d = dict[
    1 => 'abc',
    2 => 'def',
  ];
  return $f($d[$k]);
}

function keyset_foo($f, $k) :mixed{
  $d = keyset[1, 2];
  return $f($d[$k]);
}


<<__EntryPoint>>
function main_crash_16258693() :mixed{
var_dump(vec_foo(func1<>, 1));
var_dump(dict_foo(func1<>, 2));
var_dump(keyset_foo(func1<>, 2));
}
