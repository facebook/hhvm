<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// bad: require is can only be used with non-abstract classes

<<file:__EnableUnstableFeatures('require_class')>>

abstract class C {}

interface I {}

trait Tr {}

trait T1 {
  require class C;
}

trait T2 {
  require class I;
}

trait T3 {
  require class Tr;
}
