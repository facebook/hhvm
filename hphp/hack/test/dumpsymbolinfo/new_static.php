<?hh

<<__ConsistentConstruct>>
abstract class A {
  public function foo(): void {}

  public static function bar(): void {
    $x = new static();
    $x->foo();
    echo 'test';
  }
}
