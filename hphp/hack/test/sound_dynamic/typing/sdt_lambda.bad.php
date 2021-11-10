<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('upcast_expression')>>

// Not SDT
class C { }

// Explicit SDT lambda, unenforced explicit parameter type
function explicit_nonenforced_nondynamic():void {
  $f = <<__SupportDynamicType>> (vec<C> $x) ==> new C();
}
