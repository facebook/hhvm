<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function func1() { return dict['c' => null]; }
function func2($x) { return $x; }
function func3() {
  return func2(sizeof(func1()['c'] ?? dict[]));
}

<<__EntryPoint>>
function main() { var_dump(func3()); }
