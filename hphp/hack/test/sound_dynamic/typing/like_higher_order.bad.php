<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class MyVec<<<__RequireDynamic>> T> {
  public function exists((function(T):bool) $f):bool { return false; }
}

<<__SupportDynamicType>>
class C {
  public function pred():bool { return false; }
}

// If $x : C is inferred, then we can't call through the dynamic side, since
// C -> bool is not a subtype of dynamic. If dynamic is inferred, then the
// call could in principle be made to work, but it is not clear that we want
// to infer different types for $x when calling through a like typed method
// exists.
function test(~MyVec<C> $v):void {
  $v->exists($x ==> $x->pred());
}
