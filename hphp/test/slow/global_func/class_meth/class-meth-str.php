<?hh

function p($m) { var_dump(implode($m, '::')); }

trait T {
  require extends Foo;
  function h() {
    return static::f();
  }
}

class Foo {
  function e() {
    return class_meth(self::class, 'f');
  }
  static function f() {
    return class_meth(static::class, 'g');
  }
  static function g() {
    die("FAIL in ".__METHOD__);
  }
}

class Bar extends Foo {
  use T;
  static function g() {
    var_dump(42);
    return class_meth(__CLASS__, __FUNCTION__);
  }
}

class Baz extends Bar {
  static function f() {
    return class_meth(parent::class, 'f');
  }
  static function g() {
    die("FAIL in ".__METHOD__);
  }
}

class Q<reify T as Foo> {
  function j() {
    return class_meth(T::class, 'f');
  }
}

<<__EntryPoint>>
function main() {
  p((new Foo)->e());
  p((new Bar)->e());
  p((new Baz)->e());
  p((new Baz)->h());
  p(Bar::f());
  p(Baz::f());
  p((new Q<Baz>)->j()()()());
  echo "Done\n";
}
