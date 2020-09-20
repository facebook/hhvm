<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function bar():int { return 3; }
}
function foo($y = 'a', $z = new C()):void {
  // Parameters get types "_ | string" and "_ | C"
  // This is illegal, because we try and add a string to an integer
  $yy = $y + 3;
  // This is legal
  $zz = $z->bar();
}
