<?hh

class MyClass {
  <<__Memoize, __MemoizeLSB>>
  public static function foo(): int {
    return 1;
  }
}
