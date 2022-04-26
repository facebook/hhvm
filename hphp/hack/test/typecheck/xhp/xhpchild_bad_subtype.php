////file1.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

newtype N1 = C;
newtype N2 as C = C;

class C implements XHPChild { }
////file2.php
<?hh
function foo(XHPChild $_):void { }
function test1(N1 $x):void {
  // Should be rejected
  foo($x);
}
function test2(N2 $y):void {
  // Should be accepted
  foo($y);
}
