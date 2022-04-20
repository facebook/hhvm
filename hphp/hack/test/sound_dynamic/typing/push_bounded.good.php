<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class C<+T> { }

function foo<T as C<~int>>(T $x):~C<int> {
  return $x;
}
