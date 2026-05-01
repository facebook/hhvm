<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file: __EnableUnstableFeatures('representable_as')>>

namespace HH\Runtime {
  <<__GatedByFeatureFlag('representable_as')>>
  newtype RepresentableAs<+T> = T;

  <<__GatedByFeatureFlag('representable_as')>>
  function reveal<T>(RepresentableAs<T> $x)[]: T;
}
