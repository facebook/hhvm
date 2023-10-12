<?hh // strict

class C {}

abstract class A1 {
  protected C $c; // responsibility of subclasses

  abstract protected function __construct();
}

abstract class A2 {
  private C $c;

  abstract protected function __construct();
}
