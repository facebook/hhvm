<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class Inv<T> { }

function test1(Inv<dynamic> $c):~Inv<int> {
  return $c;
}
