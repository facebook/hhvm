<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Can be marked covariant
interface IGetter<T> {
  public function get():T;
}
// Can be marked contravariant
interface ISetter<T> {
  public function set(T $x):void;
}
// Cannot be marked either
interface K<T> extends ISetter<T> {
  public function foo(): void;
  public function get():T;
}
// Reified, so do not check
abstract class CRGetter< <<__Soft>> reify T> {
  abstract public function get():T;
}
// Reified, so do not check
abstract class CRSetter<reify T> {
  abstract public function set(T $x):void;
}
class C<T> {
  public function __construct(private T $item) { }
  // Doesn't affect variance cos it's private
  private function redherring(T $x):void { }
  public function get():T { return $this->item; }
}
class D<T> {
  public function __construct(private T $item) { }
  public function get():int { return 3; }
}
