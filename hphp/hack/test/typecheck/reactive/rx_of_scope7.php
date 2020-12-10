<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx>>
  public function f(int $a): void {
    // OK - rx of scope
    $a = () ==> 1;
    // OK
    $this->g($a);
  }

  <<__Rx>>
  public function g(Rx<(function(): int)> $f): void {
  }
}
