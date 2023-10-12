<?hh // strict
class Contra<-T> {
  public static final function foo(int $v): ?T {
    return null;
  }
}
final class Co<+T> {
  public static function bar(T $x): void {}
}
