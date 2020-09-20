<?hh // strict

interface Rx {
  <<__Rx>>
  public function f(int $a): void;
}


class A<T> {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public function f(T $a): void {
  }
}

class B extends A<int> implements Rx {
}
