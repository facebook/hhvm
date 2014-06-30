<?php
class Foo {}
abstract class C {
  abstract public function __construct(Foo $x);
}
class D extends C {
  public function __construct(array $x) {}
}
