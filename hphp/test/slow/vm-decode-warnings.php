<?hh

class Foo {
  function f($s) { call_user_func($s); }
  function g() { echo "g()\n"; }
  static function h($s) { call_user_func($s); }
  static function i() { echo "i()\n"; }
}

<<__EntryPoint>>
function main() {
  Foo::h("static::i");
  Foo::h("static::g");
  Foo::h(["static", "i"]);
  Foo::h(["static", "g"]);
  (new Foo)->f("static::g");
  (new Foo)->f("static::i");
  (new Foo)->f(["static", "i"]);
  (new Foo)->f(["static", "g"]);
}
