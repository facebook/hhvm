<?hh // strict

interface Rx1 {
  <<__RxShallow>>
  protected function get(): int;
}

interface Rx2 {
}

class Cls {
  <<__RxLocal, __OnlyRxIfImpl(Rx2::class)>>
  final public function run(): int {
    return 1;
  }
}

class A {
  <<__RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public function f(): void {
    $a = new Cls();
    $b = $a->run();
  }
}
