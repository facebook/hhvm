<?hh

final class Foo {
  public static function bar(string $x): void {
    echo $x;
  }

  public function baz(): void {
    $x = $this::bar<>;
    $x("Hello World!\n");
  }
}

<<__EntryPoint>>
function main(): void {
  $x = new Foo();
  $x->baz();
}
