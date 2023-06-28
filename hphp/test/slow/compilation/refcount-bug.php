<?hh

class X {
  static function test() : X {
    require_once "refcount-bug.inc";
    included();
    $ret = self::foo();
    return $ret;
  }
  static function foo() :mixed{ return new X; }
}


<<__EntryPoint>>
function main_refcount_bug() :mixed{
var_dump(X::test());
}
