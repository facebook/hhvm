<?hh // strict

interface Rx1 {
}

interface Rx2 {
}

abstract class A {
  <<__RxShallowIfImplements(Rx1::class)>>
  public function mayberx(): int {
    // error - condition does not match
    return $this->mayberx2();
  }

  <<__RxShallowIfImplements(Rx2::class)>>
  public abstract function mayberx2(): int;
}
