<?hh // strict

abstract class Base {
  abstract const type ID = arraykey;
  const type ALIAS = static::ID;
}

class IntID extends Base {
  const type ID = int;
}

class Test {
  const type TC_ID = IntID;

  public function getX(): self::TC_ID::ALIAS {
    return '';
  }
}
