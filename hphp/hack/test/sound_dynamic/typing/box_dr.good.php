<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class BoxInt {
  public function __construct(private int $x) {}
  public function set(int $y) : void {$this->x = $y;}
  public function get() : int {return $this->x;}
}
