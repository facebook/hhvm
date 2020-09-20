<?hh

interface Foo {
  public static function bar(string $x): void;
}

final class Qux implements Foo {
  public static function bar(string $x): void {
    echo $x;
  }
}

function adze<reify T as Foo>(): void {
  $x = T::bar<>;
  $x("Hello World\n");
}

<<__EntryPoint>>
function main(): void {
  adze<Qux>();
}
