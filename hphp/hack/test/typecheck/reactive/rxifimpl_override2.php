<?hh // strict

interface Rx {
  <<__Rx>>
  public function f(int $a): void;
}

trait TRx implements Rx {
}

class A<T> {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public function f(T $a): void {
  }
}

class B extends A<int> {
  use TRx;
}

<<__Rx>>
function f(B $b): void {
  $b->f(1);
}
