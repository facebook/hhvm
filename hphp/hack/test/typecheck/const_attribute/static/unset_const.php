<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const dict<int, dict<int, string>> C2 =  dict[2 => dict[4 => "folly"]];
  const dict<int, string> C1 = dict[2 => "folly"];
}

<<__EntryPoint>>
function mutate_const():void {
  unset(A::C2[2][4]);
  unset(A::C1[2]);
  unset(A::C1[]);
}
