<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function dict1(int $i): dict<int, int> {
  $d = dict[];
  $d[$i] = 42;
  return $d;
}

function dict2(int $i): dict<int, int> {
  $d = dict[];
  $d[HH\FIXME\UNSAFE_CAST<int, int>($i)] = 42;
  return $d;
}

<<__EntryPoint>>
function main():void {
  $d1 = dict1(3);
  var_dump($d1);
  $d2 = dict2(3);
  var_dump($d2);
}
