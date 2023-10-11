<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Box<T> {
  public function __construct(private T $item) {}
  public function set(T $x): void { $this->item = $x;}
  public function get(): T { return $this->item; }
}

class C {
  public function foo(): void { }
}
function foo(?C $oc): void {
  $b = new Box($oc);
  $b->get()?->foo();
  $b->set(null);
}
