<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class C{}

function test(~?dict<string, C> $v) : void {}

function repro(dict<string, ~C> $v) : void {
  test($v);
}
