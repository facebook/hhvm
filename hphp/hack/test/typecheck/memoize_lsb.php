<?hh

class MyClass {
  <<__MemoizeLSB>>
  public static function foo(): int {
    return 1;
  }
}
