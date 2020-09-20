<?hh // strict

abstract class C {
  abstract const type TFoo as num;

  public function f(this::TFoo $foo): void {
    $this->h($foo);
  }

  private function g(mixed $x): void {
    if ($x is this::TFoo) {
      $this->f($x);
    }
  }

  protected function h(num $x): void {}
}
