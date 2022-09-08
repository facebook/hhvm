<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

final class PFooEnt {

  const type TData = shape('a' => int);

  public function getData(): this::TData {
    return shape('a' => 4);
  }
}

trait PFooAutogenTrait {
  require class PFooEnt;

  public function foo(): int {
    return $this->getData()['a'];
  }
}
