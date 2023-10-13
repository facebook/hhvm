<?hh

abstract class C {
  abstract const type T1;
  abstract const type T2 as this::T1;

  abstract public function getT2(): this::T2;

  public function getT1(): this::T1 {
    return $this->getT2();
  }
}
