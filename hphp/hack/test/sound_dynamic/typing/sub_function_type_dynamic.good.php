<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expectDyn(dynamic $_):void { }

<<__SoundDynamicCallable>>
class C { }

function test1(
  (function(dynamic):int) $f1,
  (function(mixed, dynamic):string) $f2,
  (function((string|dynamic),dynamic...):float) $f3,
  (function():void) $f4,
  (function():C) $f5):void {
  expectDyn($f1);
  expectDyn($f2);
  expectDyn($f3);
  expectDyn($f4);
  expectDyn($f5);
  expectDyn($x ==> $x+1);
  expectDyn((dynamic $d) ==> $d->foo());
}
