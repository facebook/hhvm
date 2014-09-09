<?php

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
  $idunno->x(Foo::$y);
  var_dump(is_string(Foo::$y));
}

go(new B);
go(new A);
