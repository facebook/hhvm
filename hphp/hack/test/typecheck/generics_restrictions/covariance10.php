<?hh // partial

class X<-T> {
  public function test(): (function(): (int, T)) {
    return $this->test();
  }
}
