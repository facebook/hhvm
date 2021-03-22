<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expectDyn(dynamic $_):void { }

class C { }
function test1(
  (function(int):int) $f1,
  (function(mixed, dynamic):mixed) $f2,
  (function():vec<mixed>) $f3,
  (function():C) $f4):void {
  expectDyn($f1);
  expectDyn($f2);
  expectDyn($f3);
  expectDyn($f4);
  expectDyn((int $x) ==> $x+1);
  expectDyn($x ==> new C());
}
