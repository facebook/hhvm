<?hh

class Foo {
  function e() {
    $m = class_meth(self::class, 'f');
    var_dump($m);
    var_dump(join($m, '::'));
  }
  function f() {
    $m = class_meth(static::class, 'g');
    var_dump($m);
    var_dump(join($m, '::'));
  }
}

class Bar extends Foo {
  function g() {}
  function h() {
    $m = class_meth(parent::class, 'f');
    var_dump($m);
    var_dump(join($m, '::'));
  }
}

<<__EntryPoint>>
function main() {
  (new Foo)->e();
  (new Bar)->f();
  (new Bar)->e();
}
