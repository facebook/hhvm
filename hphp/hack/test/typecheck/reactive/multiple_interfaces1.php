<?hh // partial

interface I1 {
  <<__RxShallow>>
  public function f(): void;
}

interface I2 extends I1 {
}

class Base {
  <<__RxShallow, __OnlyRxIfImpl(I2::class)>>
  public function f(): void {}
}

class Derived extends Base implements I2, I1 {
}
