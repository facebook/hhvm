<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const int C = 5;
}

<<__EntryPoint>>
function mutate_const():void {
  unset(A::C);
}
