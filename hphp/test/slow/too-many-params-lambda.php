<?hh

class A {
  public function get(): mixed {
    () ==> {};
    () ==> {};
    return () ==> {};
  }
}

<<__EntryPoint>>
function main(): void {
  $a = new A();
  $lambda = $a->get();
  $lambda(1, 2, 3);
}
