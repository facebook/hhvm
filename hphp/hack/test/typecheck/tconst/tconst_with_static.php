<?hh // strict

abstract class Base {
  abstract const type ID as arraykey;
  const type ALIAS = this::ID;
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
