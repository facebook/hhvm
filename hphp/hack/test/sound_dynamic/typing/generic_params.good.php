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

<<__SupportDynamicType>>
interface Getter<+T> {
  public function get(): T;
}

<<__SupportDynamicType>>
class ROBox<+T> implements Getter<vec<T>> {
  public function __construct(private vec<T> $item) { }
  public function get(): vec<T> { return $this->item; }
}

<<__SupportDynamicType>>
class KeyBox<T as arraykey> {
  public function __construct(private T $item) { }
  public function get(): T { return $this->item; }
  public function set(T $x):void { $this->item = $x; }
}

function expectDynamic(dynamic $d):void { }
function testit(Getter<float> $g):void {
  expectDynamic(new Box(3));
  expectDynamic(new Box(true));
  expectDynamic(new SimpleBox("a"));
  expectDynamic(new KeyBox<arraykey>(3));
  expectDynamic($g);
}
