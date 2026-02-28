<?hh

class B {
  protected static $x;
  function goB() :mixed{
    self::$x = 2;
    var_dump(self::$x);
  }
}
class D extends B {
  public static $x = 'constant string';
}
class Y extends D {
  function go() :mixed{
    var_dump(self::$x);
    var_dump(B::$x);
  }
}


<<__EntryPoint>>
function main_public_static_props_001() :mixed{
(new B)->goB();
(new Y)->go();
}
