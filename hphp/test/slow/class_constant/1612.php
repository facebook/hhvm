<?hh

class X {
  public $foo = Y::FOO;
  static function foo() {
    var_dump(__METHOD__);
  }
}

<<__EntryPoint>>
function main_1612() {
X::foo();
}
