<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const dict<int, string> C = dict[2 => "folly"];
}

<<__EntryPoint>>
function mutate_const():void {
  unset(A::C[2]);
}
