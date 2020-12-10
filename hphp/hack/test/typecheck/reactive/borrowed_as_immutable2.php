<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  private ?int $value;
  <<__Rx, __Mutable>>
  public function setMutable(int $i): this {
    $this->value = $i;
    // ERROR, cannot return borrowed as immutable
    return $this;
  }
}
