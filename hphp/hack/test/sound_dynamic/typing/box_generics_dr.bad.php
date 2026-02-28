<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__DynamicallyReferenced>>
class Box<T> {
  public function __construct(private T $x) {}
  public function set(T $y) : void {$this->x = $y;}
  public function get() : T {return $this->x;}
}
