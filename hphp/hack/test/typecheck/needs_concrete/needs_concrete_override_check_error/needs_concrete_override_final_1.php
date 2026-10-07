<?hh

abstract class Base {
  public static function m(): int {
    return 0;
  }
}

trait T {
  abstract public static function hook(): int;

  <<__NeedsConcrete>>
  public static function m(): int {
    return static::hook();
  }
}

final class C extends Base { // ok: final and concrete
  use T;
  public static function hook(): int {
    return 1;
  }
}

abstract final class D extends Base { // ok: no inherited __ConsistentConstruct
  use T;
  public static function hook(): int {
    return 1;
  }
}

class E extends Base { // error: not final
  use T;
  public static function hook(): int {
    return 1;
  }
}

<<__ConsistentConstruct>>
interface ICC {}

abstract final class F extends Base implements ICC { // error: not concrete
  use T;
  public static function hook(): int {
    return 1;
  }
}
