<?hh

final class Test {
  public static function foo(): void {
    echo "This method has no generics";
  }
}

<<__EntryPoint>>
function main(): void {
  $f = Test::foo<int>;
  $f();
}
