<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const dict<int, string> C = dict[1 => "green"];
}

function foo(): void{
  $a = new A();
  unset($a::C);
}
