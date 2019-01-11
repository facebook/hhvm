<?hh

class Foo {
  static $y = 'asd';
}

abstract class IDunno {
  abstract function x($z);
}
class A extends IDunno {
  function x(&$z) { $z = 2; }
}
class B extends IDunno {
  function x($z) { $z = 2; }
}

function go(IDunno $idunno) {
  var_dump(is_string(Foo::$y));
  if ($idunno is A) {
    $idunno->x(&Foo::$y);
  } else {
    $idunno->x(Foo::$y);
  }
  var_dump(is_string(Foo::$y));
}


<<__EntryPoint>>
function main_public_static_props_012() {
go(new B);
go(new A);
}
