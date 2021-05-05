<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class Box<T> {
  public function __construct(private T $item) { }
  public function get(): T { return $this->item; }
  public function set(T $x):void { $this->item = $x; }
}

<<__SupportDynamicType>>
class SimpleBox<T> {
  public function __construct(public T $item) { }
}

function expectDynamic(dynamic $d):void { }
function testit():void {
  expectDynamic(new Box<int>(3));
  expectDynamic(new SimpleBox<string>("a"));
  expectDynamic(new KeyBox<int>(3));
}

<<__SupportDynamicType>>
interface Getter<+T> {
  public function get(): T;
}

<<__SupportDynamicType>>
class AnotherBox<T> implements Getter<vec<T>> {
  public function __construct(private vec<T> $item) { }
  public function get(): vec<T> { return $this->item; }
  public function set(T $x): void { $this->item = vec[$x]; }
}

<<__SupportDynamicType>>
class KeyBox<T as arraykey> {
  public function __construct(private T $item) { }
  public function get(): T { return $this->item; }
  public function set(T $x):void { $this->item = $x; }
}
