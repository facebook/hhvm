<?hh

class Foo {
  public static function bar(string $x): void {
    echo "Incorrect\n";
  }

  public static function baz(): void {
    $x = static::bar<>;
    $x("Hello World!\n");
  }
}

class Qux extends Foo {
  public static function bar(string $x): void {
    echo $x;
  }
}

<<__EntryPoint>>
function main(): void {
  Qux::baz();
}
