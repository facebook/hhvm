<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SoundDynamicCallable>>
class C {
  // Legal, as we don't check signatures for private members
  private vec<int> $items;
  private function setItems(vec<int> $vi):void { $this->items = $vi; }

  public function __construct() { $this->items = vec[]; }
  public function getItems(): vec<int> {
    return $this->items;
  }
  public function isMember(int $x):bool {
    return false;
  }
}
