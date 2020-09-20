<?hh

class A {}
class C {
  const type T = int;
  const type Ta = A;

  function f() {
    return type_structure(static::class, 'Ta')['classname'];
  }
  static function g() {
    return type_structure(static::class, 'Ta')['classname'];
  }
}

<<__EntryPoint>>
function main() {
  $int = type_structure(C::class, 'T');
  $a = type_structure(C::class, 'Ta')['classname'];
  $a2 = C::g();
  $c = new C();
  $a3 = $c->f();
  var_dump($int, $a, $a2, $a3);
}
