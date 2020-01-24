<?hh

final class Foo {
  public function bar(int $x): void {}

  public function foo(): void{
    $x = $this->bar<>;
    $x(4);
  }
}
