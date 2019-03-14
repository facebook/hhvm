<?hh // partial

interface Y<-T> {
  public function set(T $x): void;
}

interface X<+T> {
  public function test(Y<T> $x): void;
}
