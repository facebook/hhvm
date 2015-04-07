<?hh

class Foo<-T> {
  public function bar<Tu as T, Tv super Tu>(Tu $x, Tv $y): void {}
}
