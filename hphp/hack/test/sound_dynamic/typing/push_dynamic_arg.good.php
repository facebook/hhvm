<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class Cov<+T> { }

function test1(Cov<dynamic> $c):~Cov<int> {
  return $c;
}
