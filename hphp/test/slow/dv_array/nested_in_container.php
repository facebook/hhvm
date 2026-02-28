<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo() :mixed{
  return Pair {vec[123], vec[456]};
}

<<__EntryPoint>>
function main_nested_in_container() :mixed{
var_dump(foo());
}
