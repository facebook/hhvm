<?hh // strict

abstract class A {
  public abstract function nonrx(string $foo): int;
}

final class B extends A {
  <<__Override, __RxShallow>>
  public function nonrx(string $foo): int {
    (<<__Mutable>> $foo) ==> 123;
    return 123;
  }
}
