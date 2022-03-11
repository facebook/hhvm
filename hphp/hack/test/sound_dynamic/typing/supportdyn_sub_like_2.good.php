<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class Foo<+T> { }
<<__SupportDynamicType>>
class C { }

function expectLike(~Foo<C> $_):void { }

function gotSD(supportdyn<~Foo<C>> $x):void {
  expectLike($x);
}
