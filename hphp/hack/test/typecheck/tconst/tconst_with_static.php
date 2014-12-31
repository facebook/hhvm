<?hh // strict

abstract class Base {
  abstract public type const ID = arraykey;
  public type const ALIAS = static::ID;
}

class IntID extends Base {
  public type const ID = int;
}

class Test {
  public type const TC_ID = IntID;

  public function getX(): self::TC_ID::ALIAS {
    return '';
  }
}
