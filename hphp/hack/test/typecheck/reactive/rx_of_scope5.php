<?hh
class A {

  public function f(int $a): void {
    // OK
    $a = () ==> 1;
  }
}
