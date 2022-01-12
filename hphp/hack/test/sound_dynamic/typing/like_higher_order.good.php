<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class MyVec<<<__RequireDynamic>> T> {
  public function exists((function(T):bool) $f):bool { return false; }
}

class C {
  public function pred():bool { return false; }
}

function test(~MyVec<C> $v):void {
  $v->exists($x ==> $x->pred());
}
