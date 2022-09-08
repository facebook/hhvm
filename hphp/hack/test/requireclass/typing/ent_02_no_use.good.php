<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

abstract class PEntBase {
  abstract const type TData;

  abstract public function getData(): this::TData;
}

final class PFooEnt extends PEntBase {

  const type TData = shape('a' => int);

  <<__Override>>
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
