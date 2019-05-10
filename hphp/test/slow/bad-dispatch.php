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
  try {
    (new Foo)->b();
  } catch (BadMethodCallException $e) {
    var_dump($e->getMessage());
  }
}
