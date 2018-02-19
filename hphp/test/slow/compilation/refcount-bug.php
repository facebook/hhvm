<?hh

class X {
  static function test() : X {
    require_once "refcount-bug.inc";
    $ret = self::foo();
    return $ret;
  }
  static function foo() { return new X; }
}

var_dump(X::test());
