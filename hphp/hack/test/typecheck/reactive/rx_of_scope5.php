<?hh // partial

class A {
  <<__Rx>>
  public function f(int $a): void {
    // OK
    $a = () ==> 1;
  }
}
