<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

class A { }

<<__SupportDynamicType>>
function foo(vec<int> $vi):A {
  return new A();
}

<<__SupportDynamicType>>
function testlam():void {
  $f = (int $x) : A ==> { return new A(); };
  $g = (int $x) ==> { return new A(); /*hh_show_env();*/ };
  $h = (vec<int> $x) ==> { return new A(); };
}
