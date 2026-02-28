<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const dict<int, dict<int, string>> C2 =  dict[2 => dict[4 => "folly"]];
}

<<__EntryPoint>>
function mutate_const():void {
  unset(A::C[2][4]);

}
