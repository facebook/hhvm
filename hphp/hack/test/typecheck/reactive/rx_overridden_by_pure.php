<?hh
interface IPure {

  public function rx(): void;


  public function rx_local(): void;


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

  public function rx(): void {}


  public function rx_local(): void {}


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
