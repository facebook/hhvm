<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expectLike<T as dynamic>(~T $_):void { }

<<__SupportDynamicType>>
function bar(int $x):int {
  return $x+1;
}

<<__SupportDynamicType>>
function testit():void {
  expectLike($x ==> bar($x));
}
