<?hh // partial

class A {
  <<__Rx>>
  public function f(int $a): void {
    // OK - rx
    $a = <<__RxOfScope>> () ==> 1;
    // OK
    $this->g($a);
  }

  <<__Rx>>
  public function g(Rx<(function(): int)> $f): void {
  }
}
