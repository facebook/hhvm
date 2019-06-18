<?hh // partial

class Foo<-T> {
  /* HH_FIXME[4110] */
  public function bar<Tu super T>(): Tu {
  }
}
