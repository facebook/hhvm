<?hh

interface MyIface {
  public static function foo(): int;
}

trait MyTrait {
  require implements MyIface;
  public static function bar(): int {
    return self::foo();
  }
}

class UseTrait implements MyIface {
  use MyTrait;
  public static function foo(): int { return 1; }
}
