<?hh

class A {
  public function a(): void {}
  public function b(): void {}
}

class B extends A {
  public function b(): void {}
  public function c(): void {}
}

interface C {
  public function c(): void;
}

interface D extends C {
  public function d(): void;
}

class E implements C {
  public function c(): void {}
}

class F extends E implements D {
  public function d(): void {}
}
