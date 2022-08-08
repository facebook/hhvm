<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

namespace HH {
  newtype supportdyn<+T> as T = (T & dynamic);
}
