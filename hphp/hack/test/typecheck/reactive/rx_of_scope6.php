<?hh

class A {
  <<__Rx>>
  public function f(int $a): void {
    // ERROR
    $a = <<__Rx, __RxOfScope>> () ==> 1;
  }
}
