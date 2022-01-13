<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function id(vec<int> $x):vec<int> {
  return $x;
}

function bar():vec<int> {
  $x = vec[2];
  (id($x))[] = 3;
  return $x;
}

<<__EntryPoint>>
function main():void {
  var_dump(bar());
}
