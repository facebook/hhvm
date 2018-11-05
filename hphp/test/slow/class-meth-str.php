<?hh

class Foo {
  function f() {
    $m = class_meth(static::class, 'g');
    var_dump($m);
    var_dump(join($m, '::'));
  }
}

class Bar extends Foo {
  function g() {}
}

<<__EntryPoint>>
function main() {
  (new Bar)->f();
}
