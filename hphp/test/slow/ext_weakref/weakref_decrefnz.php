<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Test {
  public static ?stdClass $stash = null;

  static function get(int $count): WeakRef<stdClass> {
    $obj = new stdClass();
    $weakref = new WeakRef($obj);
    self::stash($count, $obj);
    return $weakref;
  }

  static function stash(int $count, stdClass $obj): void {
    if ($count !== 0) {
      self::$stash = $obj;
    }
  }
}

<<__EntryPoint>>
function main() :mixed{
  for ($i = 99; $i >= 0; $i--) {
    $weakref = Test::get($i);
    var_dump($weakref->valid());
    var_dump($weakref->get());
  }
}
