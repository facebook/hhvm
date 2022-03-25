<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait T1 {
  require class C;
}

trait T2 {
  require class C;
  require class D;
}

class C {}
class D {}
