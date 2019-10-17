<?hh

class Foo {
  function bar() { include 'this-in-pseudomain.inc'; }
  static function baz() { include 'this-in-pseudomain.inc'; }
}

<<__EntryPoint>>
function main() {
  (new Foo)->bar();
  Foo::baz();
  include 'this-in-pseudomain.inc';
}
