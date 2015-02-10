<?hh // strict

class C {
  const type X = int;

  public function foo(this::X $x): void {}
}
