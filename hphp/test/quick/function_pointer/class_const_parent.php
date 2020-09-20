<?hh

class Foo {
  public static function bar(string $x): void {
    echo $x;
  }
}

class Baz extends Foo {
  public static function bar(string $x): void {
    echo "Incorrect\n";
  }

  public static function qux(): void {
    $x = parent::bar<>;
    $x("Hello World!\n");
  }
}

<<__EntryPoint>>
function main(): void {
  Baz::qux();
}
