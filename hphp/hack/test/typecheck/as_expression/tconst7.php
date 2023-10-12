<?hh // strict

abstract class C {
  abstract const type TFoo as num;

  public function f(this::TFoo $foo): void {
    $this->h($foo);
  }

  private function g1(mixed $x): void {
    $this->f($x as this::TFoo);
  }

  private function g2(mixed $x): void {
    $this->f($x ?as this::TFoo);
  }

  protected function h(num $x): void {}
}
