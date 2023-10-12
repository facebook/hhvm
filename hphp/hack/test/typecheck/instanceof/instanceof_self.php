<?hh // strict

class D {
  public function g(): void {
    echo __METHOD__, "\n";
  }
}

class C extends D {

  public function g(): void {
    echo __METHOD__, "\n";
  }

  public static function test(mixed $x): void {
    if ($x is self) {
      hh_show($x);
      $x->g();
    }
    return;
  }
}
