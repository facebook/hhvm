<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function foo(mixed $x):int {
  return HH\FIXME\UNSAFE_CAST<?int, int>(HH\FIXME\UNSAFE_CAST<mixed,?int>($x));
}

<<__EntryPoint>>
function main():void {
  var_dump(foo(3));
}
