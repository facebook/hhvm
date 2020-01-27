<?hh

class Qux {
  public static function baz(string $x): void {
    echo "Incorrect\n";
  }
}

final class Foo extends Qux {
  public static function baz(string $x): void {
    echo $x;
  }

  public function bar(): void {
    $x = self::baz<>;
    $x("Hello World!\n");
  }
}

<<__EntryPoint>>
function main(): void {
  $x = new Foo();
  $x->bar();
}
