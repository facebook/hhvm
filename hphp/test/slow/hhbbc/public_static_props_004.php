<?hh

class B {
  public static $x = 'ok';
}

final class Y extends B { // nooverride
  public static $x = 2;
}

function set(Y $y) :mixed{
  $y::$x = 42;
}

function x() :mixed{
  set(new Y);
  var_dump(is_int(Y::$x));
  var_dump(is_string(B::$x));
}


<<__EntryPoint>>
function main_public_static_props_004() :mixed{
B::$x = 'another string';

x();
}
