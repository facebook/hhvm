<?hh

trait T {
  require class C;
}

final class C  {
  use T;
  public static function foo(): void { echo "I am foo\n"; }
}

<<__EntryPoint>>
function main(): void {
  T::foo();
}
