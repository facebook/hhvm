<?hh

abstract class X {
  <<__MemoizeLSB>>
  public static abstract function f(): void;
}

final class Y extends X {
  public static function f(): void {}
}
