<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__RxLocal>>
  public function f(int $a): void {
    // OK - rx of scope
    $a = () ==> 1;
    // ERROR
    $this->g($a);
  }

  <<__Rx>>
  public function g(Rx<(function(): int)> $f): void {
  }
}
