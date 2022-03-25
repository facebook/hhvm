<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// bad: require class is only allowed inside traits

<<file:__EnableUnstableFeatures('require_class')>>

class C {}

interface IT {
  require class C;
}

class D {
  require class C;
}
