<?hh

enum E : string {
  A = "A";
}
class Foo {
  <<__Memoize>>
  public function someMethod(E $e): void {}
}

<<__Memoize>>
function some_function(?E $e): void {}
