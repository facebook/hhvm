<?hh // strict

class A implements HH\ClassAttribute {
  public function __construct(mixed $x) {}
}

<<A(shape("A" => 123))>>
class X {}

<<A(dict["x" => -123])>>
class Y {}

<<A(keyset["a", "b", "c"])>>
class Z {}

<<A(tuple(1, 2, 3))>>
class W {}
