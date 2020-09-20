<?hh

final class Foo {
  public static function bar(int $x): void {}

  public function baz(): void {
    $x = $this::bar<>;
    $x('hello');
  }
}
