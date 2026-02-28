<?hh

class FooBar extends Foo {
  static function doit() {
    var_dump(__FUNCTION__);
    parent::pmeth();
  }
}

function bar() {
  FooBar::doit();
  FooBar::doit();
  FooBar::doit();
}
