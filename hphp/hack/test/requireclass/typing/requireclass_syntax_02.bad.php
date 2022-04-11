<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// bad: require is can only be used with classes

<<file:__EnableUnstableFeatures('require_class')>>

interface I {}

trait Tr {}

trait T2 {
  require class I;
}

trait T3 {
  require class Tr;
}
