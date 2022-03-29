<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expect1(~supportdyn<(function(~int):string)> $f):void { }

function test1(supportdyn<(function(~int):~string)> $f):void {
  expect1($f);
}

function expect2(~(function():void) $f):void { }
function test2((function():~void) $f):void {
  expect2($f);
}
