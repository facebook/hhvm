<?hh

class A {}
class C {
  const type T = int;
  const type Ta = A;

  function f() :mixed{
    return type_structure(static::class, 'Ta')['classname'];
  }
  static function g() :mixed{
    return type_structure(static::class, 'Ta')['classname'];
  }
}

<<__EntryPoint>>
function main() :mixed{
  $int = type_structure(C::class, 'T');
  $a = type_structure(C::class, 'Ta')['classname'];
  $a2 = C::g();
  $c = new C();
  $a3 = $c->f();
  var_dump($int, $a, $a2, $a3);
}
