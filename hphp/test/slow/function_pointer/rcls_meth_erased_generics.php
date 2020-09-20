<?hh

final class Test {
  public static function foo<T>(): void {
    echo "This method has erased generics";
  }
}

<<__EntryPoint>>
function main(): void {
  $f = Test::foo<int>;
  $f();
}
