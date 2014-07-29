<?hh

class A {}
class B extends A {}

interface I1 {
  public function f(): Continuation<A>;
}

interface I2 extends I1 {
  public function f(): Continuation<B>;
}
