<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class Box<T> {
  public function __construct(private T $item) { }
  public function set(T $x):void { $this->item = $x; }
  public function get(): T { return $this->item; }
}
