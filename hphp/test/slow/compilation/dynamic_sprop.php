<?hh

class Base {
  static $x = 'asd';
}

class Derived extends Base {
}

function getstr() { return 'x'; }

function heh() {
  $foo = getstr();
  Derived::${$foo} = 2;
  var_dump(Base::$x);
}

heh();
