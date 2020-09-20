<?hh // strict

interface I<+T> {
  public function get(): T;
}

class A<+T> implements I<T> {
  public function get(): T {
    // UNSAFE
  }
}

trait T {
  require extends A<mixed>;
}

class B extends A<int> {
  use T;
}

class C extends B {}
