<?hh

interface IPure {
  <<__Rx>>
  public function rx(): void;

  <<__RxLocal>>
  public function rx_local(): void;

  <<__RxShallow>>
  public function rx_shallow(): void;
}
class IPureChild implements IPure {
  <<__Pure>>
  public function rx(): void {}

  <<__Pure>>
  public function rx_local(): void {}

  <<__Pure>>
  public function rx_shallow(): void {}
}

class Base {
  <<__Rx>>
  public function rx(): void {}

  <<__RxLocal>>
  public function rx_local(): void {}

  <<__RxShallow>>
  public function rx_shallow(): void {}
}
class Child extends Base {
  <<__Pure, __Override>>
  public function rx(): void {}

  <<__Pure, __Override>>
  public function rx_local(): void {}

  <<__Pure, __Override>>
  public function rx_shallow(): void {}
}
