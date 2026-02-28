<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function get1() :mixed{
  if (__hhvm_intrinsics\launder_value(true)) {
    return vec['a', 'b', 'c'];
  } else {
    return vec[];
  }
}
function get2() :mixed{
  if (__hhvm_intrinsics\launder_value(true)) {
    return dict[0 => 'a', 1 => 'b', 2 => 'c'];
  } else {
    return dict[];
  }
}

function foo() :mixed{ return get1() === get2(); }

<<__EntryPoint>>
function main_hhbbc2() :mixed{
var_dump(foo());
}
