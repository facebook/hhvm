<?hh

abstract class C1 {
  public function f(this $_): void {
  }
}

abstract class Test {
  private C1 $c1;

  public function fInstance(): void {
    $this->c1->f($this->c1);
    $c1 = $this->c1;
    $c1->f($c1);
  }

  public static function fStatic(C1 $c1): void {
    $c1->f($c1);
  }
}
