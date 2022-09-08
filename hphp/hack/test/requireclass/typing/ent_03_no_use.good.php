<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

final class PFooEnt {

  const type TData = shape('a' => int);
}

trait PFooAutogenTrait {
  require class PFooEnt;

  const type MyData = this::TData;

  public function gen(): this::MyData {
    return shape('a' => 5);
  }
}
