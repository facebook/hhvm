<?hh // partial

class A {}
class B extends A {}

interface I1 {
  public function f(): Generator<int, A, void>;
}

interface I2 extends I1 {
  public function f(): Generator<int, B, void>;
}
