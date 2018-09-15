<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo() {
  return Pair {varray[123], varray[456]};
}

<<__EntryPoint>>
function main_nested_in_container() {
var_dump(foo());
}
