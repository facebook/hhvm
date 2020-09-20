<?hh

final class Foo {
  private static function bar(string $x): void {
    echo $x;
  }

  public function baz(): void {
    $x = self::bar<>;
    $x("Hello World!\n");
  }
}

<<__EntryPoint>>
function main(): void {
  $x = new Foo();
  $x->baz();
}
