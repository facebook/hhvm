<?hh

class X {
  static function test() : X {
    require_once "refcount-bug.inc";
    $ret = self::foo();
    return $ret;
  }
  static function foo() { return new X; }
}


<<__EntryPoint>>
function main_refcount_bug() {
var_dump(X::test());
}
