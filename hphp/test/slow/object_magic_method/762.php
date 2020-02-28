<?hh

class Foo {
  public static $bar;
  public function __call($fun, $args) {
    self::$bar = varray[$fun];
  }
}
function getName($part) {
  return 'funny_'.$part;
}
function scoped($foo, $name) {
  call_user_func(varray[$foo, getName($name)]);
}

<<__EntryPoint>>
function main_762() {
scoped(new Foo, 'a');
var_dump(Foo::$bar);
}
