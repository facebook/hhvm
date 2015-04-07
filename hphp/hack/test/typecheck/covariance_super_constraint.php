<?hh

class Foo<-T> {
  public function bar<Tu super T>(): Tu {
    // UNSAFE
  }
}
