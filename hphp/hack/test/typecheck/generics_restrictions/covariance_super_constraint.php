<?hh // partial

class Foo<-T> {
  public function bar<Tu super T>(): Tu {
    throw new Exception();
  }
}
