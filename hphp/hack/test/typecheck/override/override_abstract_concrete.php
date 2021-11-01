<?hh

class C {
  public function f(): void {}
}

abstract class D extends C {
  abstract public function f(): void;
}
