<?hh

class Bar {
  <<__MemoizeLSB("Bar")>>
  public static function foo(): void {}
}
