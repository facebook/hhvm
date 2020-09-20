<?hh // strict

class A {
  <<__Rx, __MutableReturn>>
  public static function make(): A {
    return new A();
  }

  <<__Rx>>
  public function immutableMethod(): int {
    return 42;
  }
}

<<__Rx>>
function f(): void {
  \HH\Rx\mutable(A::make())->immutableMethod();
}
