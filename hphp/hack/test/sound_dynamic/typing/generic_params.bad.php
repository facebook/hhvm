<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
class Box<<<__RequireDynamic>> T> {
  public function __construct(private T $item) { }
  public function get(): T { return $this->item; }
  public function set(T $x):void { $this->item = $x; }
}

<<__SupportDynamicType>>
class SimpleBox<<<__RequireDynamic>> T> {
  public function __construct(public T $item) { }
}

function testit():void {
  new Box<int>(3) upcast dynamic;
  new SimpleBox<string>("a") upcast dynamic;
  new KeyBox<int>(3) upcast dynamic;
}

<<__SupportDynamicType>>
interface Getter<<<__RequireDynamic>> +T> {
  public function get(): T;
}

<<__SupportDynamicType>>
class AnotherBox<<<__RequireDynamic>> T> implements Getter<vec<T>> {
  public function __construct(private vec<T> $item) { }
  public function get(): vec<T> { return $this->item; }
  public function set(T $x): void { $this->item = vec[$x]; }
}

<<__SupportDynamicType>>
class KeyBox<<<__RequireDynamic>> T as arraykey> {
  public function __construct(private T $item) { }
  public function get(): T { return $this->item; }
  public function set(T $x):void { $this->item = $x; }
}
