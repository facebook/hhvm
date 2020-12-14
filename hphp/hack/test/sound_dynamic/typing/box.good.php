<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class BoxInt implements dynamic {
  public function __construct(private int $x) {}
  public function set(int $y) : void {$this->x = $y;}
  public function get() : int {return $this->x;}
}
