<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('upcast_expression')>>

class C {
  <<__SupportDynamicType>>
  public function foo(vec<int> $x):int {
    $x upcast vec<int>;
    return $x[0];
  }
}
