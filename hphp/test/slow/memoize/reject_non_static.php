<?hh

class Foo {
  <<__Memoize>>
  public static function func1() {
    return 1;
  }

  <<__Memoize>>
  public function func2() {
    return 1;
  }

}

var_dump(HH\clear_static_memoization("Foo", "func1"));
var_dump(HH\clear_static_memoization("Foo", "func2"));
