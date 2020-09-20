<?hh

class B {
  static $x = 'ok';
}

final class Y extends B { // nooverride
  static $x = 2;
}

function set(Y $y) {
  $y::$x = 42;
}

function x() {
  set(new Y);
  var_dump(is_int(Y::$x));
  var_dump(is_string(B::$x));
}


<<__EntryPoint>>
function main_public_static_props_004() {
B::$x = 'another string';

x();
}
