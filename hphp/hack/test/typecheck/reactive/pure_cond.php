<?hh // strict

class MyClass {
  <<__Pure>>
  final public static function bar(): bool { return true; }
}

abstract class MyTestClass {
  <<__Pure, __OnlyRxIfImpl(MyClass::class)>>
  abstract protected static function bar(): bool;
  <<__Pure, __OnlyRxIfImpl(MyClass::class)>>
  final public static function foo(): bool {
    return static::bar();
  }
}

abstract class MyOtherTestClass {
  <<__Rx, __OnlyRxIfImpl(MyClass::class)>>
  abstract protected static function bar(): bool;
  <<__Rx, __OnlyRxIfImpl(MyClass::class)>>
  final public static function foo(): bool {
    return static::bar();
  }
}
