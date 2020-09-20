<?hh // strict

class A {
  public function foo(): void {}

  public static function bar(): void {
    $x = new self();
    $x->foo();
  }
}
