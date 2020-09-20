<?hh // partial

class A {
  <<__RxLocal>>
  public function f(int $a): void {
    // OK - rx
    $a = <<__RxOfScope>> () ==> 1;
    // ERROR
    $this->g($a);
  }

  <<__Rx>>
  public function g(Rx<(function(): int)> $f): void {
  }
}
