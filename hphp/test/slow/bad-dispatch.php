<?hh

class Foo {
  function a() {}
  static function b() {}
}

<<__EntryPoint>>
function main() {
  (new Foo)->a();
  Foo::b();

  Foo::a();
  (new Foo)->b();
}
