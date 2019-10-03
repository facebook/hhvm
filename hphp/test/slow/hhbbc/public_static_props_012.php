<?hh

class Foo {
  static $y = 'asd';
}

abstract class IDunno {
  abstract function x($z);
}
class A extends IDunno {
  function x(inout $z) { $z = 2; }
}
class B extends IDunno {
  function x($z) { $z = 2; }
}

