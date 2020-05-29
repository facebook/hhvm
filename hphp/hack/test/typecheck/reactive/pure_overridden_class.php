<?hh

class Base {
  <<__Pure>>
  public function pure(): void {}
}
class Child extends Base {
  <<__Override>>
  public function pure(): void {}
}
