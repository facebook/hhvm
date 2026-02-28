<?hh

trait T {
  abstract static function foo():mixed;
  static function bar() :mixed{
    return static::foo<>;
  }
}
class C {
  use T;
  static function foo() :mixed{
    var_dump(__METHOD__);
  }
}

<<__EntryPoint>>
function main() :mixed{
  C::bar()();
  $f = T::bar();
  echo "FAIL\n";
  $f();
}
