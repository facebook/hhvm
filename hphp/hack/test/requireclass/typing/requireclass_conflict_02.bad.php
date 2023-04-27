<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

final class C {}

trait T {
  require class C;
  require extends C;
}
