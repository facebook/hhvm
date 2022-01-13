<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function id(dict<int,string> $x):dict<int,string> {
  return $x;
}

function bar():dict<int,string> {
  $x = dict[2 => 'a'];
  (id($x))[3] = 'b';
  return $x;
}

<<__EntryPoint>>
function main():void {
  var_dump(bar());
}
