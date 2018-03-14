<?hh // strict

interface Rx1 {
}

interface Rx2 {
}

class C {
  public function __construct(public int $a) {}
}

abstract class A {
  <<__RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function mayberx(C $c): int;
}

abstract class B extends A {
  <<__Override, __RxShallow, __OnlyRxIfImpl(Rx2::class)>>
  public abstract function mayberx(C $c): int;
}
