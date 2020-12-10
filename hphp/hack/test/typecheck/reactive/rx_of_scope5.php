<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx>>
  public function f(int $a): void {
    // OK
    $a = () ==> 1;
  }
}
