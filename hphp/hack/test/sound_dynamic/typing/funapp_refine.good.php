<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
function foo(int $x, vec<int> $y): void { }

<<__SupportDynamicType>>
function bar(int $y, ?vec<int> $z):void {
  $w = $z ?: vec[];
  foo($y, $w);
  }
