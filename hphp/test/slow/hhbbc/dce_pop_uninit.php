<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function func1() :mixed{ return dict['c' => null]; }
function func2($x) :mixed{ return $x; }
function func3() :mixed{
  return func2(sizeof(func1()['c'] ?? dict[]));
}

<<__EntryPoint>>
function main() :mixed{ var_dump(func3()); }
