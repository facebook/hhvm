<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

abstract class C {}

final abstract class D {}

trait T1 {
  require class C;
}

trait T2 {
  require class D;
}
