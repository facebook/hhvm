<?hh // partial

class Foo<-T> {
  /* HH_FIXME[4336] */
  public function bar<Tu super T>(): Tu {
  }
}
