<?hh // strict

class B {
  public function foo(): void {}
}

class A extends B {
  public static function bar(): void {
    $x = new parent();
    $x->foo();
    echo 'test';
  }
}
