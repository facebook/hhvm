<?hh

class B {
  public static $x = 2;
}
final class D extends B {
  function x(D $d) :mixed{
    $d::$x = 3;
    return $this;
  }
  function y() :mixed{
    return self::$x;
  }
}

function go() :mixed{
  $y = (new D)->x(new D)->y();
  var_dump($y);
  var_dump(is_int($y));
}


<<__EntryPoint>>
function main_public_static_props_002() :mixed{
go();
}
