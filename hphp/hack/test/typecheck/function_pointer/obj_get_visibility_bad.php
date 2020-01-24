<?hh

final class Foo {
  private function bar(int $x): void {}

  public function foo(): void{
    $x = $this->bar<>;
    $x('hello');
  }
}
