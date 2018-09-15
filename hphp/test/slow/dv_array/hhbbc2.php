<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function get1() {
  if (__hhvm_intrinsics\launder_value(true)) {
    return varray['a', 'b', 'c'];
  } else {
    return varray[];
  }
}
function get2() {
  if (__hhvm_intrinsics\launder_value(true)) {
    return darray[0 => 'a', 1 => 'b', 2 => 'c'];
  } else {
    return darray[];
  }
}

function foo() { return get1() === get2(); }

<<__EntryPoint>>
function main_hhbbc2() {
var_dump(foo());
}
